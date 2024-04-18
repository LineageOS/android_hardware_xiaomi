/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Button.h"

#define LOG_TAG "Button"

#include <android-base/logging.h>
#include <vector>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

LedButton::LedButton(std::string type) : mLed(type) {}

void LedButton::setBacklight(float value) { mLed.setBrightness(value); }

bool LedButton::exists() { return mLed.exists(); }

static const std::string kLedButtonDevices[] = {
        "button-backlight",
        "button-backlight1",
};

std::vector<ButtonDevice> getButtonDevices() {
    std::vector<ButtonDevice> devices;

    for (const auto& device : kLedButtonDevices) {
        auto button = LedButton(device);
        if (button.exists()) {
            LOG(INFO) << "Found button device " << device;
            devices.emplace_back(button);
        }
    }

    return devices;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
