/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

/**
 * A Linux LED device.
 * @param type The LED type.
 * @see https://docs.kernel.org/leds/leds-class.html
 */
class LedDevice {
  public:
    LedDevice(std::string type);

    bool exists();
    bool setBreath(float value);
    bool setBrightness(float value);

    enum Type {
        RED,
        GREEN,
        BLUE,
        WHITE,
        MAX_LED,
    };

    static LedDevice kDefaultLeds[Type::MAX_LED];

  private:
    std::string mBasePath;
    uint32_t mMaxBrightness;
    bool mBreath;

    static const uint32_t kDefaultMaxBrightness;
    static const std::string kBaseLedsPath;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
