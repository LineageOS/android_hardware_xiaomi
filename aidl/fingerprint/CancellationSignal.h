/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>

#include "Session.h"

namespace aidl::android::hardware::biometrics::fingerprint {

class CancellationSignal : public ::aidl::android::hardware::biometrics::common::BnCancellationSignal {
  public:
    CancellationSignal(Session* session);
    ndk::ScopedAStatus cancel() override;

  private:
    Session* mSession;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
