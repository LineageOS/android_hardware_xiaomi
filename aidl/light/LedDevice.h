/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <string>
#include "IDumpable.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

enum LightMode {
    STATIC,
    BREATH,
    TIMED,
};

/**
 * A Linux LED device.
 * @see https://docs.kernel.org/leds/leds-class.html
 */
class LedDevice : public IDumpable {
  public:
    LedDevice() = delete;

    /**
     * Constructor.
     *
     * @param name The name of the LED device
     */
    LedDevice(std::string name);

    /**
     * Get the name of the LED device.
     *
     * @return std::string The name of the LED device
     */
    std::string getName() const;

    /**
     * Return whether this LED device exists.
     *
     * @return bool true if the LED device exists, false otherwise
     */
    bool exists() const;

    /**
     * Return whether this LED device supports breathing.
     * When it doesn't, calling setBrightness with LightMode::BREATH will behave like
     * LightMode::STATIC.
     *
     * @return bool true if the LED device supports breathing, false otherwise
     */
    bool supportsBreath() const;

    /**
     * Return whether this LED device supports timed mode.
     * When it doesn't, calling setBrightness with LightMode::TIMED will behave like
     * LightMode::BREATH.
     *
     * @return bool true if the LED device supports timed mode, false otherwise
     */
    bool supportsTimed() const;

    /**
     * Set the brightness of the LED device.
     *
     * @param value The brightness value to set
     * @param mode The light mode to use
     * @return bool true if the brightness was set successfully, false otherwise
     */
    bool setBrightness(uint8_t value, LightMode mode = LightMode::STATIC, uint32_t flashOnMs = 0,
                       uint32_t flashOffMs = 0);

    /**
     * Set the index of the LED device.
     *
     * @param idx The index to set
     */
    void setIdx(int idx);

    void dump(int fd) const override;

  private:
    std::string mName;
    int mIdx;
    std::string mBasePath;
    uint32_t mMaxBrightness;
    std::string mBreathNode;
    bool mSupportsTimed;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
