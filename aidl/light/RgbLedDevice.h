/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "LedDevice.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

/**
 * A pool of LED devices that will be toggled based on the wanted color.
 * Support all types of LED combinations, with a maximum of 3 LEDs.
 * Also supports 2 color LEDs (*cough* ASUS *cough*).
 */
class RgbLedDevice {
  public:
    RgbLedDevice(LedDevice red, LedDevice green, LedDevice blue);

    bool exists() const;
    bool setBrightness(rgb color, LightMode mode = LightMode::STATIC);

    enum Color {
        NONE = 0,
        RED = 1 << 0,
        GREEN = 1 << 1,
        BLUE = 1 << 2,
        ALL = RED | GREEN | BLUE,
    };

  private:
    LedDevice mRed;
    LedDevice mGreen;
    LedDevice mBlue;

    int mColors;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
