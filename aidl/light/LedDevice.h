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
 * @param type The LED type.
 * @see https://docs.kernel.org/leds/leds-class.html
 */
class LedDevice {
  public:
    LedDevice(std::string type);
    virtual ~LedDevice() = default;

    virtual bool exists();
    virtual bool setBrightness(float value, LightMode mode = LightMode::STATIC);

  private:
    std::string mBasePath;
    uint32_t mMaxBrightness;
    bool mBreath;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
