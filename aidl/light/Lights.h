/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <mutex>

using ::aidl::android::hardware::light::HwLightState;
using ::aidl::android::hardware::light::HwLight;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight> *_aidl_return) override;
private:
    void setLED(const HwLightState& state);

    std::vector<HwLight> mLights;

    std::string mBacklightPath;
    std::vector<std::string> mButtonsPaths;
    bool mWhiteLED;

    std::mutex mLEDMutex;
    HwLightState mLastBatteryState;
    HwLightState mLastNotificationState;
};

} // namespace light
} // namespace hardware
} // namespace android
} // namespace aidl
