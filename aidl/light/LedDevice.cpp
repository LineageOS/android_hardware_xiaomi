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

static const std::string kBreathNodes[] = {
        "breath",
        "blink",
};

LedDevice::LedDevice(std::string name) : mName(name), mBasePath(kBaseLedsPath + name + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }

    for (const auto& node : kBreathNodes) {
        if (std::ifstream(mBasePath + node).good()) {
            mBreathNode = node;
            break;
        }
    }
}

std::string LedDevice::getName() const {
    return mName;
}

bool LedDevice::supportsBreath() const {
    return !mBreathNode.empty();
}

bool LedDevice::exists() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

bool LedDevice::setBrightness(uint8_t value, LightMode mode) {
    // Disable current blinking
    if (supportsBreath()) {
        writeToFile(mBasePath + mBreathNode, 0);
    }

    switch (mode) {
        case LightMode::BREATH:
            if (supportsBreath()) {
                return writeToFile(mBasePath + mBreathNode, value > 0 ? 1 : 0);
                break;
            }

            // Fallthrough to static mode if breath is not supported
            FALLTHROUGH_INTENDED;
        case LightMode::STATIC:
            return writeToFile(mBasePath + kBrightnessNode, scaleBrightness(value, mMaxBrightness));
            break;
        default:
            LOG(ERROR) << "Unknown mode: " << mode;
            return false;
            break;
    }
}

void LedDevice::dump(int fd) const {
    dprintf(fd, "Name: %s", mName.c_str());
    dprintf(fd, ", exists: %d", exists());
    dprintf(fd, ", base path: %s", mBasePath.c_str());
    dprintf(fd, ", max brightness: %u", mMaxBrightness);
    dprintf(fd, ", supports breath: %d", supportsBreath());
    dprintf(fd, ", breath node: %s", mBreathNode.c_str());
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
