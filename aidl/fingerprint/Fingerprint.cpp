/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Fingerprint.h"

#include <android-base/properties.h>
#include <fingerprint.sysprop.h>
#include "util/Util.h"

#include <android-base/logging.h>
#include <android-base/strings.h>

namespace aidl::android::hardware::biometrics::fingerprint {

namespace {
constexpr char HW_COMPONENT_ID[] = "fingerprintSensor";
constexpr char HW_VERSION[] = "vendor/model/revision";
constexpr char FW_VERSION[] = "1.01";
constexpr char SERIAL_NUMBER[] = "00000001";
constexpr char SW_COMPONENT_ID[] = "matchingAlgorithm";
constexpr char SW_VERSION[] = "vendor/version/revision";
}  // namespace

static const uint16_t kVersion = HARDWARE_MODULE_API_VERSION(2, 1);
static Fingerprint* sInstance;

Fingerprint::Fingerprint() {
    sInstance = this;  // keep track of the most recent instance

    if (mDevice) {
        ALOGI("fingerprint HAL already opened");
    } else {
        std::string sensorModulesList = Fingerprint::cfg().get<std::string>("sensor_modules");
        std::vector<std::string> sensorModules = ::android::base::Split(sensorModulesList, ",");
        for (const std::string& class_name : sensorModules) {
            mDevice = openSensorHal(class_name.c_str());
            if (!mDevice) {
                ALOGE("Can't open HAL module, class %s", class_name.c_str());
                continue;
            }
            ALOGI("Opened fingerprint HAL, class %s", class_name.c_str());
            break;
        }
        if (!mDevice) {
            ALOGE("Can't open any fingerprint HAL module");
        }
    }

    std::string sensorTypeProp = Fingerprint::cfg().get<std::string>("type");
    if (sensorTypeProp == "udfps" || sensorTypeProp == "udfps_optical") {
        if (sensorTypeProp == "udfps") {
            mSensorType = FingerprintSensorType::UNDER_DISPLAY_ULTRASONIC;
        } else {
            mSensorType = FingerprintSensorType::UNDER_DISPLAY_OPTICAL;
        }
        mUdfpsHandlerFactory = getUdfpsHandlerFactory();
        if (!mUdfpsHandlerFactory) {
            ALOGE("Can't get UdfpsHandlerFactory");
        } else {
            mUdfpsHandler = mUdfpsHandlerFactory->create();
            if (!mUdfpsHandler) {
                ALOGE("Can't create UdfpsHandler");
            } else {
                mUdfpsHandler->init(mDevice);
            }
        }
    } else if (sensorTypeProp == "side") {
        mSensorType = FingerprintSensorType::POWER_BUTTON;
    } else if (sensorTypeProp == "home") {
        mSensorType = FingerprintSensorType::HOME_BUTTON;
    } else if (sensorTypeProp == "rear") {
        mSensorType = FingerprintSensorType::REAR;
    } else {
        mSensorType = FingerprintSensorType::UNKNOWN;
        UNIMPLEMENTED(FATAL) << "unrecognized or unimplemented fingerprint behavior: "
                             << sensorTypeProp;
    }
    ALOGI("sensorTypeProp: %s", sensorTypeProp.c_str());
}

Fingerprint::~Fingerprint() {
    ALOGV("~Fingerprint()");
    if (mUdfpsHandler) {
        mUdfpsHandlerFactory->destroy(mUdfpsHandler);
    }
    if (mDevice == nullptr) {
        ALOGE("No valid device");
        return;
    }
    int err;
    if (0 != (err = mDevice->common.close(reinterpret_cast<hw_device_t*>(mDevice)))) {
        ALOGE("Can't close fingerprint module, error: %d", err);
        return;
    }
    mDevice = nullptr;
}

fingerprint_device_t* Fingerprint::openSensorHal(const char* class_name) {
    const hw_module_t* hw_mdl = nullptr;

    ALOGD("Opening fingerprint hal library...");
    if (hw_get_module_by_class(FINGERPRINT_HARDWARE_MODULE_ID, class_name, &hw_mdl) != 0) {
        ALOGE("Can't open fingerprint HW Module");
        return nullptr;
    }

    if (!hw_mdl) {
        ALOGE("No valid fingerprint module");
        return nullptr;
    }

    auto module = reinterpret_cast<const fingerprint_module_t*>(hw_mdl);
    if (!module->common.methods->open) {
        ALOGE("No valid open method");
        return nullptr;
    }

    hw_device_t* device = nullptr;
    if (module->common.methods->open(hw_mdl, nullptr, &device) != 0) {
        ALOGE("Can't open fingerprint methods");
        return nullptr;
    }

    auto fp_device = reinterpret_cast<fingerprint_device_t*>(device);
    if (fp_device->set_notify(fp_device, Fingerprint::notify) != 0) {
        ALOGE("Can't register fingerprint module callback");
        return nullptr;
    }

    return fp_device;
}

SensorLocation Fingerprint::getSensorLocation() {
    SensorLocation location;

    auto loc = Fingerprint::cfg().get<std::string>("sensor_location");
    auto isValidStr = false;
    auto dim = ::android::base::Split(loc, "|");

    if (dim.size() != 3 and dim.size() != 4) {
        if (!loc.empty()) {
            ALOGE("Invalid sensor location input (x|y|radius) or (x|y|radius|display): %s",
                  loc.c_str());
        }
    } else {
        int32_t x, y, r;
        std::string d = "";
        isValidStr = ParseInt(dim[0], &x) && ParseInt(dim[1], &y) && ParseInt(dim[2], &r);
        if (dim.size() == 4) {
            d = dim[3];
            isValidStr = isValidStr && !d.empty();
        }
        if (isValidStr)
            location = {
                    .sensorLocationX = x, .sensorLocationY = y, .sensorRadius = r, .display = d};
    }

    return location;
}

void Fingerprint::notify(const fingerprint_msg_t* msg) {
    Fingerprint* thisPtr = sInstance;
    if (thisPtr == nullptr || thisPtr->mSession == nullptr || thisPtr->mSession->isClosed()) {
        ALOGE("Receiving callbacks before a session is opened.");
        return;
    }
    thisPtr->mSession->notify(msg);
}

ndk::ScopedAStatus Fingerprint::getSensorProps(std::vector<SensorProps>* out) {
    std::vector<common::ComponentInfo> componentInfo = {
            {HW_COMPONENT_ID, HW_VERSION, FW_VERSION, SERIAL_NUMBER, "" /* softwareVersion */},
            {SW_COMPONENT_ID, "" /* hardwareVersion */, "" /* firmwareVersion */,
             "" /* serialNumber */, SW_VERSION}};
    auto sensorId = Fingerprint::cfg().get<std::int32_t>("sensor_id");
    auto sensorStrength = Fingerprint::cfg().get<std::int32_t>("sensor_strength");
    auto maxEnrollments = Fingerprint::cfg().get<std::int32_t>("max_enrollments");
    auto navigationGuesture = Fingerprint::cfg().get<bool>("navigation_guesture");
    auto detectInteraction = Fingerprint::cfg().get<bool>("detect_interaction");
    auto displayTouch = Fingerprint::cfg().get<bool>("display_touch");
    auto controlIllumination = Fingerprint::cfg().get<bool>("control_illumination");

    common::CommonProps commonProps = {sensorId, (common::SensorStrength)sensorStrength,
                                       maxEnrollments, componentInfo};

    SensorLocation sensorLocation = getSensorLocation();

    ALOGI("sensor type: %s, location: %s", ::android::internal::ToString(mSensorType).c_str(),
          sensorLocation.toString().c_str());

    *out = {{commonProps,
             mSensorType,
             {sensorLocation},
             navigationGuesture,
             detectInteraction,
             displayTouch,
             controlIllumination,
             std::nullopt}};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Fingerprint::createSession(int32_t /*sensorId*/, int32_t userId,
                                              const std::shared_ptr<ISessionCallback>& cb,
                                              std::shared_ptr<ISession>* out) {
    CHECK(mSession == nullptr || mSession->isClosed()) << "Open session already exists!";

    mSession = SharedRefBase::make<Session>(mDevice, mUdfpsHandler, userId, cb, mLockoutTracker);
    *out = mSession;

    mSession->linkToDeath(cb->asBinder().get());

    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
