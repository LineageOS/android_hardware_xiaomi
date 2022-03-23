/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/hardware/biometrics/fingerprint/2.3/IBiometricsFingerprint.h>
#include <hardware/fingerprint.h>
#include <hardware/hardware.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/xiaomi/hardware/fingerprintextension/1.0/IXiaomiFingerprint.h>
#include "xiaomi_fingerprint.h"

namespace android {
namespace hardware {
namespace biometrics {
namespace fingerprint {
namespace V2_3 {
namespace implementation {

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintAcquiredInfo;
using ::android::hardware::biometrics::fingerprint::V2_1::FingerprintError;
using ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprintClientCallback;
using ::android::hardware::biometrics::fingerprint::V2_1::RequestStatus;
using ::android::hardware::biometrics::fingerprint::V2_3::IBiometricsFingerprint;
using ::vendor::xiaomi::hardware::fingerprintextension::V1_0::IXiaomiFingerprint;

struct BiometricsFingerprint : public IBiometricsFingerprint, public IXiaomiFingerprint {
  public:
    BiometricsFingerprint();
    ~BiometricsFingerprint();

    // Method to wrap legacy HAL with BiometricsFingerprint class
    template <class T>
    static T* getInstance() {
      if (!sInstance) {
        sInstance = new BiometricsFingerprint();
      }
      return sInstance;
    }

    // Methods from ::android::hardware::biometrics::fingerprint::V2_1::IBiometricsFingerprint
    // follow.
    Return<uint64_t> setNotify(
            const sp<IBiometricsFingerprintClientCallback>& clientCallback) override;
    Return<uint64_t> preEnroll() override;
    Return<RequestStatus> enroll(const hidl_array<uint8_t, 69>& hat, uint32_t gid,
                                 uint32_t timeoutSec) override;
    Return<RequestStatus> postEnroll() override;
    Return<uint64_t> getAuthenticatorId() override;
    Return<RequestStatus> cancel() override;
    Return<RequestStatus> enumerate() override;
    Return<RequestStatus> remove(uint32_t gid, uint32_t fid) override;
    Return<RequestStatus> setActiveGroup(uint32_t gid, const hidl_string& storePath) override;
    Return<RequestStatus> authenticate(uint64_t operationId, uint32_t gid) override;

    // Methods from ::android::hardware::biometrics::fingerprint::V2_3::IBiometricsFingerprint
    // follow.
    Return<bool> isUdfps(uint32_t sensorId) override;
    Return<void> onFingerDown(uint32_t x, uint32_t y, float minor, float major) override;
    Return<void> onFingerUp() override;

    // Methods from ::vendor::xiaomi::hardware::fingerprintextension::V1_0::IXiaomiFingerprint
    // follow.
    Return<int32_t> extCmd(int32_t cmd, int32_t param) override;

  private:
    static xiaomi_fingerprint_device_t* openHal(const char* class_name);
    static void notify(
            const fingerprint_msg_t* msg); /* Static callback for legacy HAL implementation */
    static Return<RequestStatus> ErrorFilter(int32_t error);
    static FingerprintError VendorErrorFilter(int32_t error, int32_t* vendorCode);
    static FingerprintAcquiredInfo VendorAcquiredFilter(int32_t error, int32_t* vendorCode);
    static BiometricsFingerprint* sInstance;

    std::mutex mClientCallbackMutex;
    sp<IBiometricsFingerprintClientCallback> mClientCallback;
    xiaomi_fingerprint_device_t* mDevice;
    bool mFod;
};

}  // namespace implementation
}  // namespace V2_3
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace hardware
}  // namespace android
