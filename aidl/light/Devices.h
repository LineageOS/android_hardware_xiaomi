/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>
#include "BacklightDevice.h"
#include "LedDevice.h"
#include "RgbLedDevice.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Devices {
  public:
    Devices();

    bool hasBacklightDevices();
    bool hasButtonDevices();
    bool hasNotificationDevices();

    void setBacklightColor(rgb color);
    void setButtonsColor(rgb color);
    void setNotificationColor(rgb color, LightMode mode = LightMode::STATIC);

  private:
    // Backlight
    std::vector<BacklightDevice> mBacklightDevices;
    std::vector<LedDevice> mBacklightLedDevices;

    // Buttons
    std::vector<LedDevice> mButtonLedDevices;

    // Notifications
    RgbLedDevice mRgbLedDevice;
    LedDevice mWhiteLed;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
