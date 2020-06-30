/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/power/BnPower.h>
#include <perfmgr/HintManager.h>

using ::aidl::vendor::lineage::power::Boost;
using ::aidl::vendor::lineage::power::Feature;
using ::android::perfmgr::HintManager;

namespace aidl {
namespace vendor {
namespace lineage {
namespace power {

enum PowerProfile {
    POWER_SAVE = 0,
    BALANCED,
    HIGH_PERFORMANCE,
    BIAS_POWER_SAVE,
    BIAS_PERFORMANCE,
    MAX
};

class LineagePower : public BnPower {
public:
    LineagePower(std::shared_ptr<HintManager> hm);

    ndk::ScopedAStatus getFeature(Feature feature, int32_t *_aidl_return) override;
    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override;
private:
    std::shared_ptr<HintManager> mHintManager;
    std::atomic<PowerProfile> mCurrentPerfProfile;

    void setProfile(PowerProfile profile);
};

} // namespace power
} // namespace lineage
} // namespace vendor
} // namespace aidl
