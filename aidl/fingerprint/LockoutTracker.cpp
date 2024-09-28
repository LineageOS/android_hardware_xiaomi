/*
 * Copyright (C) 2022 The Android Open Source Project
 *               2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LockoutTracker.h"
#include <fingerprint.sysprop.h>
#include "Fingerprint.h"
#include "util/Util.h"

using namespace ::android::fingerprint::xiaomi;

namespace aidl::android::hardware::biometrics::fingerprint {

void LockoutTracker::reset(bool clearAttemptCounter) {
    if (clearAttemptCounter)
        mFailedCount = 0;
    mLockoutTimedStart = 0;
    mCurrentMode = LockoutMode::kNone;
}

void LockoutTracker::addFailedAttempt() {
    bool enabled = Fingerprint::cfg().get<bool>("lockout_enable");
    if (enabled) {
        mFailedCount++;
        int32_t lockoutTimedThreshold =
                Fingerprint::cfg().get<std::int32_t>("lockout_timed_threshold");
        int32_t lockoutPermanetThreshold =
                Fingerprint::cfg().get<std::int32_t>("lockout_permanent_threshold");
        if (mFailedCount >= lockoutPermanetThreshold) {
            mCurrentMode = LockoutMode::kPermanent;
            Fingerprint::cfg().set<bool>("lockout", true);
        } else if (mFailedCount >= lockoutTimedThreshold) {
            if (mCurrentMode == LockoutMode::kNone) {
                mCurrentMode = LockoutMode::kTimed;
                mLockoutTimedStart = Util::getSystemNanoTime();
            }
        }
    } else {
        reset(true);
    }
}

LockoutTracker::LockoutMode LockoutTracker::getMode() {
    if (mCurrentMode == LockoutMode::kTimed) {
        int32_t lockoutTimedDuration =
                Fingerprint::cfg().get<std::int32_t>("lockout_timed_duration");
        if (Util::hasElapsed(mLockoutTimedStart, lockoutTimedDuration)) {
            mCurrentMode = LockoutMode::kNone;
            mLockoutTimedStart = 0;
        }
    }

    return mCurrentMode;
}

int64_t LockoutTracker::getLockoutTimeLeft() {
    int64_t res = 0;

    if (mLockoutTimedStart > 0) {
        int32_t lockoutTimedDuration =
                Fingerprint::cfg().get<std::int32_t>("lockout_timed_duration");
        auto now = Util::getSystemNanoTime();
        auto elapsed = (now - mLockoutTimedStart) / 1000000LL;
        res = lockoutTimedDuration - elapsed;
        LOG(INFO) << "elapsed=" << elapsed << " now = " << now
                  << " mLockoutTimedStart=" << mLockoutTimedStart << " res=" << res;
    }

    return res;
}

}  // namespace aidl::android::hardware::biometrics::fingerprint
