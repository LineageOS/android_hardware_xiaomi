/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BacklightDevice.h"

#define LOG_TAG "BacklightDevice"

#include <android-base/logging.h>
#include <vector>
#include "LedDevice.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

BacklightBrightness::BacklightBrightness(std::string name)
        : mBasePath(kBacklightBasePath + name + "/") {
    if (!readFromFile(mBasePath + "max_brightness", &mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }
};

void BacklightBrightness::setBacklight(float value) {
    writeToFile(mBasePath + "brightness", value * mMaxBrightness);
}

bool BacklightBrightness::exists() { return fileWriteable(mBasePath + "brightness"); }

static const std::string BacklightBrightness::kBacklightBasePath = "/sys/class/backlight/";

static const uint32_t BacklightBrightness::kDefaultMaxBrightness = 255;

LedBacklight::LedBacklight(std::string type) : mLed(type) {};

void LedBacklight::setBacklight(uint8_t value) { mLed.setBrightness(value); }

bool LedBacklight::exists() { return mLed.exists(); }

static const std::string kBacklightDevices[] = {
        "backlight",
        "panel0-backlight",
};

static const std::string kLedDevices[] = {
        "lcd-backlight",
};

std::vector<BacklightDevice> getBacklightDevices() {
    std::vector<BacklightDevice> devices;

    for (auto& device : kBacklightDevices) {
        auto backlight = BacklightBrightness(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found backlight device " << device;
            devices.push_back(backlight);
        }
    }

    for (auto& device : kLedDevices) {
        auto backlight = LedBacklight(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found LED backlight device " << device;
            devices.push_back(backlight);
        }
    }

    return devices;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
