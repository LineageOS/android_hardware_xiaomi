/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "RgbLedDevice.h"

#define LOG_TAG "RgbLedDevice"

#include <android-base/logging.h>
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

RgbLedDevice::RgbLedDevice(LedDevice red, LedDevice green, LedDevice blue, std::string rgbSyncNode)
    : mRed(red), mGreen(green), mBlue(blue), mRgbSyncNode(rgbSyncNode), mColors(Color::NONE) {
    if (mRed.exists()) {
        mColors |= Color::RED;
    }
    if (mGreen.exists()) {
        mColors |= Color::GREEN;
    }
    if (mBlue.exists()) {
        mColors |= Color::BLUE;
    }
    if (supportsRgbSync()) {
        mRed.setIdx(0);
        mGreen.setIdx(1);
        mBlue.setIdx(2);
    }
}

bool RgbLedDevice::exists() const {
    return mColors != Color::NONE;
}

bool RgbLedDevice::supportsBreath() const {
    return (!mRed.exists() || mRed.supportsBreath()) &&
           (!mGreen.exists() || mGreen.supportsBreath()) &&
           (!mBlue.exists() || mBlue.supportsBreath());
}

bool RgbLedDevice::supportsTimed() const {
    return (!mRed.exists() || mRed.supportsTimed()) &&
           (!mGreen.exists() || mGreen.supportsTimed()) &&
           (!mBlue.exists() || mBlue.supportsTimed());
}

bool RgbLedDevice::supportsRgbSync() const {
    return std::ifstream(mRgbSyncNode).good();
}

bool RgbLedDevice::setBrightness(rgb color, LightMode mode, uint32_t flashOnMs,
                                 uint32_t flashOffMs) {
    bool rc = true;

    if (mColors == Color::NONE) {
        LOG(ERROR) << "No LEDs found";
        return false;
    }

    if (mode == LightMode::TIMED && !supportsTimed()) {
        // Not all LEDs support timed mode, force breathing mode
        mode = LightMode::BREATH;
    }

    if (mode == LightMode::BREATH && !supportsBreath()) {
        // Not all LEDs support breathing, force static mode
        mode = LightMode::STATIC;
    }

    if (mode == LightMode::TIMED && supportsRgbSync()) {
        rc &= writeToFile(mRgbSyncNode, 0);
    }

    if (mColors == Color::ALL) {
        rc &= mRed.setBrightness(color.red, mode, flashOnMs, flashOffMs);
        rc &= mGreen.setBrightness(color.green, mode, flashOnMs, flashOffMs);
        rc &= mBlue.setBrightness(color.blue, mode, flashOnMs, flashOffMs);
    } else {
        // Check if we have only one LED
        if (mColors == Color::RED) {
            rc &= mRed.setBrightness(color.toBrightness(), mode, flashOnMs, flashOffMs);
        } else if (mColors == Color::GREEN) {
            rc &= mGreen.setBrightness(color.toBrightness(), mode, flashOnMs, flashOffMs);
        } else if (mColors == Color::BLUE) {
            rc &= mBlue.setBrightness(color.toBrightness(), mode, flashOnMs, flashOffMs);
        } else {
            // We only have two LEDs, blend the missing color in the other two
            if ((mColors & Color::RED) == Color::NONE) {
                rc &= mBlue.setBrightness((color.blue + color.red) / 2, mode, flashOnMs,
                                          flashOffMs);
                rc &= mGreen.setBrightness((color.green + color.red) / 2, mode, flashOnMs,
                                           flashOffMs);
            } else if ((mColors & Color::GREEN) == Color::NONE) {
                rc &= mRed.setBrightness((color.red + color.green) / 2, mode, flashOnMs,
                                         flashOffMs);
                rc &= mBlue.setBrightness((color.blue + color.green) / 2, mode, flashOnMs,
                                          flashOffMs);
            } else if ((mColors & Color::BLUE) == Color::NONE) {
                rc &= mRed.setBrightness((color.red + color.blue) / 2, mode, flashOnMs, flashOffMs);
                rc &= mGreen.setBrightness((color.green + color.blue) / 2, mode, flashOnMs,
                                           flashOffMs);
            }
        }
    }

    if (mode == LightMode::TIMED && supportsRgbSync()) {
        rc &= writeToFile(mRgbSyncNode, 1);
    }

    return rc;
}

void RgbLedDevice::dump(int fd) const {
    dprintf(fd, "Exists: %d", exists());
    dprintf(fd, ", supports breath: %d", supportsBreath());
    dprintf(fd, ", supports timed: %d", supportsTimed());
    dprintf(fd, ", supports RGB sync: %d", supportsRgbSync());
    dprintf(fd, ", colors:");
    if (mColors != Color::NONE) {
        if (mColors & Color::RED) {
            dprintf(fd, "\nRed: ");
            mRed.dump(fd);
        }
        if (mColors & Color::GREEN) {
            dprintf(fd, "\nGreen: ");
            mGreen.dump(fd);
        }
        if (mColors & Color::BLUE) {
            dprintf(fd, "\nBlue: ");
            mBlue.dump(fd);
        }
    } else {
        dprintf(fd, " None");
    }
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
