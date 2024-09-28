/*
 * Copyright (C) 2022 The Android Open Source Project
 *               2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android/binder_to_string.h>
#include <stdint.h>
#include <string>

namespace aidl::android::hardware::biometrics::fingerprint {

class LockoutTracker {
  public:
    LockoutTracker() : mFailedCount(0) {}
    ~LockoutTracker() {}

    enum class LockoutMode : int8_t { kNone = 0, kTimed, kPermanent };

    void reset(bool clearAttemptCounter);
    LockoutMode getMode();
    void addFailedAttempt();
    int64_t getLockoutTimeLeft();
    inline std::string toString() const {
        std::ostringstream os;
        os << "----- LockoutTracker:: -----" << std::endl;
        os << "LockoutTracker::mFailedCount:" << mFailedCount;
        os << ", LockoutTracker::mCurrentMode:" << (int)mCurrentMode;
        os << std::endl;
        return os.str();
    }

  private:
    int32_t mFailedCount;
    int64_t mLockoutTimedStart;
    LockoutMode mCurrentMode;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
