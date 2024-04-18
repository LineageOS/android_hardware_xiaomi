/*
 * Copyright (C) 2022-2024 The LineageOS Project
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

BacklightDevice::BacklightDevice(std::string name)
    : mName(name), mBasePath(kBacklightBasePath + name + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }
};

std::string BacklightDevice::getName() const {
    return mName;
}

bool BacklightDevice::exists() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool BacklightDevice::setBrightness(uint8_t value) {
    return writeToFile(mBasePath + kBrightnessNode, scaleBrightness(value, mMaxBrightness));
}

void BacklightDevice::dump(int fd) const {
    dprintf(fd, "Name: %s", mName.c_str());
    dprintf(fd, ", exists: %d", exists());
    dprintf(fd, ", base path: %s", mBasePath.c_str());
    dprintf(fd, ", max brightness: %u", mMaxBrightness);
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
