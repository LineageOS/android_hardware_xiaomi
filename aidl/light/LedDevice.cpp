/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LedDevice.h"

#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

LedDevice::LedDevice(std::string type) : mBasePath(kBaseLedsPath + type + "/") {
    if (!readFromFile(mBasePath + "max_brightness", mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }

    mBreath = fileWriteable(mBasePath + "breath");
}

bool LedDevice::exists() {
    return fileWriteable(mBasePath + "brightness");
}

bool LedDevice::setBreath(float value) {
    return writeToFile(mBasePath + (mBreath ? "breath" : "blink"), value * mMaxBrightness);
}

bool LedDevice::setBrightness(float value) {
    return writeToFile(mBasePath + "brightness", value * mMaxBrightness / 0xFF);
}

/*
static LedDevice LedDevice::kDefaultLeds[] = {
        [RED] = LedDevice("red"),
        [GREEN] = LedDevice("green"),
        [BLUE] = LedDevice("blue"),
        [WHITE] = LedDevice("white"),
};
*/

static const uint32_t LedDevice::kDefaultMaxBrightness = 255;

static const std::string LedDevice::kBaseLedsPath = "/sys/class/leds/";

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
