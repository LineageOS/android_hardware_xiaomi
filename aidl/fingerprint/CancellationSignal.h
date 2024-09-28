/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/biometrics/common/BnCancellationSignal.h>

#include "Session.h"

using ::aidl::android::hardware::biometrics::common::BnCancellationSignal;

namespace aidl::android::hardware::biometrics::fingerprint {

class CancellationSignal : public BnCancellationSignal {
  public:
    CancellationSignal(Session* session);
    ndk::ScopedAStatus cancel() override;

  private:
    Session* mSession;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
