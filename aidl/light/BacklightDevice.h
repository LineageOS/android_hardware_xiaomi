/*
 * Copyright (C) 2022 The LineageOS Project
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

/**
 * A Linux backlight device.
 * @see https://www.kernel.org/doc/Documentation/ABI/stable/sysfs-class-backlight
 */
class BacklightDevice {
  public:
    BacklightDevice(std::string name);

    bool exists();
    void setBrightness(uint8_t value);

  private:
    std::string mBasePath;
    uint32_t mMaxBrightness;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
