/*
 * Copyright (C) 2024-2025 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Fingerprint.h"

#include <android-base/properties.h>
#include <fingerprint.sysprop.h>
#include "util/Util.h"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

using ::android::base::SetProperty;

namespace aidl::android::hardware::biometrics::fingerprint {

namespace {
constexpr int MAX_ENROLLMENTS_PER_USER = 5;
constexpr char HW_COMPONENT_ID[] = "fingerprintSensor";
constexpr char HW_VERSION[] = "vendor/model/revision";
constexpr char FW_VERSION[] = "1.01";
constexpr char SERIAL_NUMBER[] = "00000001";
constexpr char SW_COMPONENT_ID[] = "matchingAlgorithm";
constexpr char SW_VERSION[] = "vendor/version/revision";

typedef struct fingerprint_hal {
    const char* class_name;
} fingerprint_hal_t;

static const fingerprint_hal_t kModules[] = {
        {"fortsense"},  {"fpc"},         {"fpc_fod"}, {"goodix"}, {"goodix:gf_fingerprint"},
        {"goodix_fod"}, {"goodix_fod6"}, {"silead"},  {"syna"},
};

}  // namespace

static const uint16_t kVersion = HARDWARE_MODULE_API_VERSION(2, 1);
static Fingerprint* sInstance;

Fingerprint::Fingerprint(std::shared_ptr<FingerprintConfig> config) : mConfig(std::move(config)) {
    sInstance = this;  // keep track of the most recent instance

    if (mDevice) {
        ALOGI("fingerprint HAL already opened");
    } else {
        for (auto& [module] : kModules) {
            std::string class_name;
            std::string class_module_id;

            auto parts = ::android::base::Split(module, ":");

            if (parts.size() == 2) {
                class_name = parts[0];
                class_module_id = parts[1];
            } else {
                class_name = module;
                class_module_id = FINGERPRINT_HARDWARE_MODULE_ID;
            }

            mDevice = openFingerprintHal(class_name.c_str(), class_module_id.c_str());
            if (!mDevice) {
                ALOGE("Can't open HAL module, class: %s, module_id: %s", class_name.c_str(),
                      class_module_id.c_str());
                continue;
            }
            ALOGI("Opened fingerprint HAL, class: %s, module_id: %s", class_name.c_str(),
                  class_module_id.c_str());
            SetProperty("persist.vendor.sys.fp.vendor", class_name);
            break;
        }
        if (!mDevice) {
            ALOGE("Can't open any fingerprint HAL module");
            SetProperty("persist.vendor.sys.fp.vendor", "none");
        }
    }

    std::string sensorTypeProp = mConfig->get<std::string>("type");
    if (sensorTypeProp == "udfps" || sensorTypeProp == "udfps_optical") {
        SetProperty("ro.hardware.fp.udfps", "true");

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

fingerprint_device_t* Fingerprint::openFingerprintHal(const char* class_name,
                                                      const char* module_id) {
    const hw_module_t* hw_mdl = nullptr;

    ALOGD("Opening fingerprint hal library...");
    if (hw_get_module_by_class(module_id, class_name, &hw_mdl) != 0) {
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

std::vector<SensorLocation> Fingerprint::getSensorLocations() {
    std::vector<SensorLocation> locations;

    auto loc = mConfig->get<std::string>("sensor_location");
    auto entries = ::android::base::Split(loc, ",");

    for (const auto& entry : entries) {
        auto isValidStr = false;
        auto dim = ::android::base::Split(entry, "|");

        if (dim.size() != 3 and dim.size() != 4) {
            if (!loc.empty()) {
                ALOGE("Invalid sensor location input (x|y|radius) or (x|y|radius|display): %s",
                      loc.c_str());
            }
        } else {
            int32_t x, y, r;
            std::string d;
            isValidStr = ParseInt(dim[0], &x) && ParseInt(dim[1], &y) && ParseInt(dim[2], &r);
            if (dim.size() == 4) {
                d = dim[3];
                isValidStr = isValidStr && !d.empty();
            }
            if (isValidStr)
                locations.push_back({.sensorLocationX = x,
                                     .sensorLocationY = y,
                                     .sensorRadius = r,
                                     .display = d});
        }
    }

    return locations;
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
    auto sensorId = mConfig->get<std::int32_t>("sensor_id");
    auto sensorStrength = mConfig->get<std::int32_t>("sensor_strength");
    auto navigationGuesture = mConfig->get<bool>("navigation_gesture");
    auto detectInteraction = mConfig->get<bool>("detect_interaction");
    auto displayTouch = mConfig->get<bool>("display_touch");
    auto controlIllumination = mConfig->get<bool>("control_illumination");

    common::CommonProps commonProps = {sensorId, (common::SensorStrength)sensorStrength,
                                       MAX_ENROLLMENTS_PER_USER, componentInfo};

    std::vector<SensorLocation> sensorLocations = getSensorLocations();

    std::vector<std::string> sensorLocationStrings;
    std::transform(sensorLocations.begin(), sensorLocations.end(),
                   std::back_inserter(sensorLocationStrings),
                   [](const SensorLocation& obj) { return obj.toString(); });

    ALOGI("sensor type: %s, location: %s", ::android::internal::ToString(mSensorType).c_str(),
          ::android::base::Join(sensorLocationStrings, ", ").c_str());

    *out = {{commonProps, mSensorType, sensorLocations, navigationGuesture, detectInteraction,
             displayTouch, controlIllumination, std::nullopt}};
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
