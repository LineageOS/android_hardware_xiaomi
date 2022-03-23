/*
 * Copyright (C) 2017 The Android Open Source Project
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "android.hardware.biometrics.fingerprint@2.3-service.xiaomi"

#include <android-base/logging.h>
#include <android/hardware/biometrics/fingerprint/2.2/types.h>
#include <android/hardware/biometrics/fingerprint/2.3/IBiometricsFingerprint.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <vendor/xiaomi/hardware/fingerprintextension/1.0/IXiaomiFingerprint.h>
#include "BiometricsFingerprint.h"

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::biometrics::fingerprint::V2_3::IBiometricsFingerprint;
using android::hardware::biometrics::fingerprint::V2_3::implementation::BiometricsFingerprint;
using vendor::xiaomi::hardware::fingerprintextension::V1_0::IXiaomiFingerprint;

int main() {
    android::sp<IBiometricsFingerprint> bio =
            BiometricsFingerprint::getInstance<IBiometricsFingerprint>();
    android::sp<IXiaomiFingerprint> xfe = BiometricsFingerprint::getInstance<IXiaomiFingerprint>();

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (bio != nullptr) {
        if (::android::OK != bio->registerAsService()) {
            return 1;
        }
    } else {
        LOG(ERROR) << "Can't create instance of BiometricsFingerprint, nullptr";
    }

    if (xfe != nullptr) {
        if (::android::OK != xfe->registerAsService()) {
            return 1;
        }
    } else {
        ALOGE("Can't create instance of XiaomiFingerprint, nullptr");
    }

    joinRpcThreadpool();

    return 0;  // should never get here
}
