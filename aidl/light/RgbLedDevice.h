/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "IDumpable.h"
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
class RgbLedDevice : public IDumpable {
  public:
    RgbLedDevice() = delete;

    /**
     * Constructor.
     *
     * @param red The red LED device
     * @param green The green LED device
     * @param blue The blue LED device
     * @param rgbSyncNode The path to RGB sync trigger
     */
    RgbLedDevice(LedDevice red, LedDevice green, LedDevice blue, std::string rgbSyncNode);

    /**
     * Return whether this RGB LED device exists.
     * This is true when at least one of the LEDs exists.
     *
     * @return bool true if the RGB LED device exists, false otherwise
     */
    bool exists() const;

    /**
     * Return whether this RGB LED device supports breathing.
     * This is true when all existing LEDs support breathing.
     * In case this is false, calling setBrightness with LightMode::BREATH will behave like
     * LightMode::STATIC.
     *
     * @return bool true if the RGB LED device supports breathing, false otherwise
     */
    bool supportsBreath() const;

    /**
     * Return whether this RGB LED device supports timed mode.
     * This is true when all existing LEDs support timed mode.
     * In case this is false, calling setBrightness with LightMode::TIMED will behave like
     * LightMode::BREATH.
     *
     * @return bool true if the RGB LED device supports timed mode, false otherwise
     */
    bool supportsTimed() const;

    /**
     * Return whether this RGB LED device supports RGB sync.
     *
     * @return bool true if the RGB LED device supports RGB sync, false otherwise
     */
    bool supportsRgbSync() const;

    /**
     * Set the brightness of this RGB LED device.
     *
     * @param color The color to set
     * @param mode The mode to set
     * @return bool true if the brightness was set successfully, false otherwise
     */
    bool setBrightness(rgb color, LightMode mode = LightMode::STATIC, uint32_t flashOnMs = 0,
                       uint32_t flashOffMs = 0);

    void dump(int fd) const override;

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
    std::string mRgbSyncNode;

    int mColors;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
