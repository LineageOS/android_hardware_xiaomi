/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <mutex>
#include "BacklightDevice.h"
#include "LedDevice.h"
#include "RgbLedDevice.h"

using ::aidl::android::hardware::light::HwLight;
using ::aidl::android::hardware::light::HwLightState;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* _aidl_return) override;

  private:
    std::vector<HwLight> mLights;

    // Backlight
    std::vector<BacklightDevice> mBacklightDevices;
    std::vector<LedDevice> mBacklightLedDevices;

    // Buttons
    std::vector<LedDevice> mButtonLedDevices;

    // Notification
    RgbLedDevice mRgbLedDevice;
    LedDevice mWhiteLed;

    HwLightState mLastBatteryState;
    HwLightState mLastNotificationsState;
    HwLightState mLastAttentionState;
    std::mutex mLedMutex;

    void updateLeds();
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
