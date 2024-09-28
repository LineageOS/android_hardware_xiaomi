/*
 * Copyright (C) 2024 The LineageOS Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/biometrics/fingerprint/BnFingerprint.h>

#include "FingerprintConfig.h"
#include "LockoutTracker.h"
#include "Session.h"
#include "UdfpsHandler.h"

using ::aidl::android::hardware::biometrics::fingerprint::ISession;
using ::aidl::android::hardware::biometrics::fingerprint::ISessionCallback;
using ::aidl::android::hardware::biometrics::fingerprint::SensorProps;
using ::aidl::android::hardware::biometrics::fingerprint::FingerprintSensorType;

namespace aidl::android::hardware::biometrics::fingerprint {

class Fingerprint : public BnFingerprint {
public:
    Fingerprint();
    ~Fingerprint();

    ndk::ScopedAStatus getSensorProps(std::vector<SensorProps>* _aidl_return) override;
    ndk::ScopedAStatus createSession(int32_t sensorId, int32_t userId,
                                     const std::shared_ptr<ISessionCallback>& cb,
                                     std::shared_ptr<ISession>* out) override;

    static FingerprintConfig& cfg() {
        static FingerprintConfig* cfg = nullptr;
        if (cfg == nullptr) {
            cfg = new FingerprintConfig();
            cfg->init();
        }
        return *cfg;
    }

private:
    static fingerprint_device_t* openSensorHal(const char* class_name);
    fingerprint_device_t* getSensorHal();
    static SensorLocation getSensorLocation();
    static void notify(const fingerprint_msg_t* msg);

    std::shared_ptr<Session> mSession;
    LockoutTracker mLockoutTracker;
    FingerprintSensorType mSensorType;

    fingerprint_device_t* mDevice;
    UdfpsHandlerFactory* mUdfpsHandlerFactory;
    UdfpsHandler* mUdfpsHandler;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
