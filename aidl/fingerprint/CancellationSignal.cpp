/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "CancellationSignal.h"

namespace aidl::android::hardware::biometrics::fingerprint {

CancellationSignal::CancellationSignal(Session* session)
    : mSession(session) {
}

ndk::ScopedAStatus CancellationSignal::cancel() {
    return mSession->cancel();
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
