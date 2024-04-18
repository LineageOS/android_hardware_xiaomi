/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BacklightDevice.h"

#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightBasePath = "/sys/class/backlight/";
static const uint32_t kDefaultMaxBrightness = 255;

BacklightDevice::BacklightDevice(std::string name) : mBasePath(kBacklightBasePath + name + "/") {
    if (!readFromFile(mBasePath + "max_brightness", mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }
};

bool BacklightDevice::exists() {
    return fileWriteable(mBasePath + "brightness");
}

void BacklightDevice::setBrightness(float value) {
    writeToFile(mBasePath + "brightness", value * mMaxBrightness);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
