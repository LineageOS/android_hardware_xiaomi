/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LedDevice.h"

#define LOG_TAG "LedDevice"

#include <android-base/logging.h>
#include <fstream>
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const uint32_t kDefaultMaxBrightness = 255;

static const std::string kBaseLedsPath = "/sys/class/leds/";

static const std::string kBrightnessNode = "brightness";
static const std::string kMaxBrightnessNode = "max_brightness";
static const std::string kBreathNode = "breath";
static const std::string kBlinkNode = "blink";

LedDevice::LedDevice(std::string type) : mBasePath(kBaseLedsPath + type + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        LOG(ERROR) << type << ": Failed to read max brightness, using " << kDefaultMaxBrightness;
        mMaxBrightness = kDefaultMaxBrightness;
    }

    mBreathNode = std::ifstream(mBasePath + kBreathNode).good() ? kBreathNode : kBlinkNode;
}

bool LedDevice::exists() {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool LedDevice::setBrightness(uint8_t value, LightMode mode) {
    uint32_t brightness = value / 255 * mMaxBrightness;
    switch (mode) {
        case LightMode::STATIC:
            return writeToFile(mBasePath + kBrightnessNode, brightness);
            break;
        case LightMode::BREATH:
            return writeToFile(mBasePath + mBreathNode, brightness);
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
