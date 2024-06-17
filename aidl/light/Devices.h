/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>
#include "BacklightDevice.h"
#include "IDumpable.h"
#include "LedDevice.h"
#include "RgbLedDevice.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Devices : public IDumpable {
  public:
    Devices();

    bool hasBacklightDevices() const;
    bool hasButtonDevices() const;
    bool hasNotificationDevices() const;

    void setBacklightColor(rgb color);
    void setButtonsColor(rgb color);
    void setNotificationColor(rgb color, LightMode mode = LightMode::STATIC, uint32_t flashOnMs = 0,
                              uint32_t flashOffMs = 0);

    void dump(int fd) const override;

  private:
    // Backlight
    std::vector<BacklightDevice> mBacklightDevices;
    std::vector<LedDevice> mBacklightLedDevices;

    // Buttons
    std::vector<LedDevice> mButtonLedDevices;

    // Notifications
    std::vector<RgbLedDevice> mNotificationRgbLedDevices;
    std::vector<LedDevice> mNotificationLedDevices;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
