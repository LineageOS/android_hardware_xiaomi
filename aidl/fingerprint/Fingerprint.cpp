/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Fingerprint.h"

namespace aidl::android::hardware::biometrics::fingerprint {

ndk::ScopedAStatus Fingerprint::getSensorProps(std::vector<SensorProps>* /*out*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Fingerprint::createSession(int32_t /*sensorId*/, int32_t /*userId*/,
                                              const std::shared_ptr<ISessionCallback>& /*cb*/,
                                              std::shared_ptr<ISession>* /*out*/) {
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
