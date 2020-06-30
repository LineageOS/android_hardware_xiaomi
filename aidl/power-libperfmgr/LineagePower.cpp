/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LineagePower.h"

#define LOG_TAG "android.hardware.power-service.xiaomi-libperfmgr"

#include <android-base/properties.h>
#include <utils/Log.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace power {

constexpr char kPowerHalProfileProp[] = "vendor.powerhal.perf_profile";

LineagePower::LineagePower(std::shared_ptr<HintManager> hm)
    : mHintManager(hm),
      mCurrentPerfProfile(PowerProfile::BALANCED) {
    std::string state = android::base::GetProperty(kPowerHalProfileProp, "");

    if (state == "POWER_SAVE") {
        ALOGI("Initialize with POWER_SAVE profile");
        setProfile(PowerProfile::POWER_SAVE);
        mCurrentPerfProfile = PowerProfile::POWER_SAVE;
    } else if (state == "BIAS_POWER_SAVE") {
        ALOGI("Initialize with BIAS_POWER_SAVE profile");
        setProfile(PowerProfile::BIAS_POWER_SAVE);
        mCurrentPerfProfile = PowerProfile::BIAS_POWER_SAVE;
    } else if (state == "BIAS_PERFORMANCE") {
        ALOGI("Initialize with BIAS_PERFORMANCE profile");
        setProfile(PowerProfile::BIAS_PERFORMANCE);
        mCurrentPerfProfile = PowerProfile::BIAS_PERFORMANCE;
    } else if (state == "HIGH_PERFORMANCE") {
        ALOGI("Initialize with HIGH_PERFORMANCE profile");
        setProfile(PowerProfile::HIGH_PERFORMANCE);
        mCurrentPerfProfile = PowerProfile::HIGH_PERFORMANCE;
    }
}

ndk::ScopedAStatus LineagePower::getFeature(Feature feature, int32_t *_aidl_return) {
    switch (feature) {
        case Feature::SUPPORTED_PROFILES:
            *_aidl_return = PowerProfile::MAX;
            break;
        default:
            *_aidl_return = -1;
            break;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus LineagePower::setBoost(Boost type, int32_t durationMs) {
    switch (type) {
        case Boost::SET_PROFILE:
            setProfile(static_cast<PowerProfile>(durationMs));
            mCurrentPerfProfile = static_cast<PowerProfile>(durationMs);
            break;
        default:
            break;
    }
    return ndk::ScopedAStatus::ok();
}

void LineagePower::setProfile(PowerProfile profile) {
    if (mCurrentPerfProfile == profile) {
        return;
    }

    // End previous perf profile hints
    switch (mCurrentPerfProfile) {
        case PowerProfile::POWER_SAVE:
            mHintManager->EndHint("PROFILE_POWER_SAVE");
            break;
        case PowerProfile::BIAS_POWER_SAVE:
            mHintManager->EndHint("PROFILE_BIAS_POWER_SAVE");
            break;
        case PowerProfile::BIAS_PERFORMANCE:
            mHintManager->EndHint("PROFILE_BIAS_PERFORMANCE");
            break;
        case PowerProfile::HIGH_PERFORMANCE:
            mHintManager->EndHint("PROFILE_HIGH_PERFORMANCE");
            break;
        default:
            break;
    }

    // Apply perf profile hints
    switch (profile) {
        case PowerProfile::POWER_SAVE:
            mHintManager->DoHint("PROFILE_POWER_SAVE");
            break;
        case PowerProfile::BIAS_POWER_SAVE:
            mHintManager->DoHint("PROFILE_BIAS_POWER_SAVE");
            break;
        case PowerProfile::BIAS_PERFORMANCE:
            mHintManager->DoHint("PROFILE_BIAS_PERFORMANCE");
            break;
        case PowerProfile::HIGH_PERFORMANCE:
            mHintManager->DoHint("PROFILE_HIGH_PERFORMANCE");
            break;
        default:
            break;
    }

    return;
}

} // namespace power
} // namespace lineage
} // namespace vendor
} // namespace aidl
