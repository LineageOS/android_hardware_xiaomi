/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LedDevice.h"

#define LOG_TAG "LedDevice"

#include <android-base/logging.h>
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const uint32_t kDefaultMaxBrightness = 255;

static const std::string kBaseLedsPath = "/sys/class/leds/";

LedDevice::LedDevice(std::string type) : mBasePath(kBaseLedsPath + type + "/") {
    if (!readFromFile(mBasePath + "max_brightness", mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }

    mBreath = fileWriteable(mBasePath + "breath");
}

bool LedDevice::exists() {
    return fileWriteable(mBasePath + "brightness");
}

bool LedDevice::setBrightness(float value, LightMode mode) {
    switch (mode) {
        case LightMode::STATIC:
            return writeToFile(mBasePath + "brightness", value * mMaxBrightness);
            break;
        case LightMode::BREATH:
            return writeToFile(mBasePath + (mBreath ? "breath" : "blink"), value * mMaxBrightness);
            break;
        default:
            LOG(ERROR) << "Unknown mode: " << mode;
            return false;
            break;
    }
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
