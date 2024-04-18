/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Devices.h"

#define LOG_TAG "Devices"

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightDevices[] = {
        "backlight",
        "panel0-backlight",
};

std::vector<BacklightDevice> getBacklightDevices() {
    std::vector<BacklightDevice> devices;

    for (const auto& device : kBacklightDevices) {
        auto backlight = BacklightDevice(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found backlight device " << device;
            devices.push_back(backlight);
        }
    }

    return devices;
}

static const std::string kLedBacklightDevices[] = {
        "lcd-backlight",
};

std::vector<LedDevice> getBacklightLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kLedBacklightDevices) {
        auto backlight = LedDevice(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found LED backlight device " << device;
            devices.push_back(backlight);
        }
    }

    return devices;
}

static const std::string kButtonLedDevices[] = {
        "button-backlight",
        "button-backlight1",
};

std::vector<LedDevice> getButtonLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kButtonLedDevices) {
        auto button = LedDevice(device);
        if (button.exists()) {
            LOG(INFO) << "Found button device " << device;
            devices.emplace_back(button);
        }
    }

    return devices;
}

RgbLedDevice getRgbLedDevice() {
    auto red = LedDevice("red");
    auto green = LedDevice("green");
    auto blue = LedDevice("blue");

    return RgbLedDevice(red, green, blue);
}

LedDevice getWhiteLedDevice() {
    return LedDevice("white");
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
