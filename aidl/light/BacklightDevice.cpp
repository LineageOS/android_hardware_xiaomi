/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BacklightDevice.h"

#define LOG_TAG "BacklightDevice"

#include <android-base/logging.h>
#include <fstream>
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightBasePath = "/sys/class/backlight/";
static const uint32_t kDefaultMaxBrightness = 255;

static const std::string kBrightnessNode = "brightness";
static const std::string kMaxBrightnessNode = "max_brightness";

BacklightDevice::BacklightDevice(std::string name) : mBasePath(kBacklightBasePath + name + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        LOG(ERROR) << name << ": Failed to read max brightness, using " << kDefaultMaxBrightness;
        mMaxBrightness = kDefaultMaxBrightness;
    }
};

bool BacklightDevice::exists() {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

void BacklightDevice::setBrightness(uint8_t value) {
    writeToFile(mBasePath + kBrightnessNode, value / 255 * mMaxBrightness);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
