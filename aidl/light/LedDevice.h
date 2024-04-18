/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

enum LightMode {
    STATIC,
    BREATH,
};

/**
 * A Linux LED device.
 * @see https://docs.kernel.org/leds/leds-class.html
 */
class LedDevice {
  public:
    LedDevice() = delete;
    LedDevice(std::string name);

    std::string getName() const;
    bool exists() const;
    bool setBrightness(uint8_t value, LightMode mode = LightMode::STATIC);

  private:
    std::string mName;
    std::string mBasePath;
    uint32_t mMaxBrightness;
    std::string mBreathNode;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
