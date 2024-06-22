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

static const std::string kBlinkNode = "blink";
static const std::string kStartIdxNode = "start_idx";
static const std::string kDutyPctsNode = "duty_pcts";
static const std::string kPauseLoNode = "pause_lo";
static const std::string kPauseHiNode = "pause_hi";
static const std::string kRampStepMsNode = "ramp_step_ms";

static constexpr int kRampSteps = 8;
static constexpr int kRampMaxStepDurationMs = 50;

LedDevice::LedDevice(std::string name)
    : mName(name), mIdx(0), mBasePath(kBaseLedsPath + name + "/") {
    if (!readFromFile(mBasePath + kMaxBrightnessNode, mMaxBrightness)) {
        mMaxBrightness = kDefaultMaxBrightness;
    }

    for (const auto& node : kBreathNodes) {
        if (std::ifstream(mBasePath + node).good()) {
            mBreathNode = node;
            break;
        }
    }

    mSupportsTimed = std::ifstream(mBasePath + kBlinkNode).good() &&
                     std::ifstream(mBasePath + kStartIdxNode).good() &&
                     std::ifstream(mBasePath + kDutyPctsNode).good() &&
                     std::ifstream(mBasePath + kPauseLoNode).good() &&
                     std::ifstream(mBasePath + kPauseHiNode).good() &&
                     std::ifstream(mBasePath + kRampStepMsNode).good();
}

std::string LedDevice::getName() const {
    return mName;
}

bool LedDevice::supportsBreath() const {
    return !mBreathNode.empty();
}

bool LedDevice::supportsTimed() const {
    return mSupportsTimed;
}

bool LedDevice::exists() const {
    return std::ifstream(mBasePath + kBrightnessNode).good();
}

static std::string getScaledDutyPercent(uint8_t brightness) {
    std::string output;
    for (int i = 0; i < kRampSteps; i++) {
        if (i != 0) {
            output += ",";
        }
        output += std::to_string(i * 100 * brightness / (0xFF * kRampSteps));
    }
    return output;
}

bool LedDevice::setBrightness(uint8_t value, LightMode mode, uint32_t flashOnMs,
                              uint32_t flashOffMs) {
    // Disable current blinking
    if (mSupportsTimed) {
        writeToFile(mBasePath + kBlinkNode, 0);
    }
    if (supportsBreath()) {
        writeToFile(mBasePath + mBreathNode, 0);
    }

    switch (mode) {
        case LightMode::TIMED:
            if (mSupportsTimed) {
                int32_t stepDuration = kRampMaxStepDurationMs;
                int32_t pauseLo = flashOffMs;
                int32_t pauseHi = flashOnMs - (stepDuration * kRampSteps * 2);

                if (pauseHi < 0) {
                    stepDuration = flashOnMs / (kRampSteps * 2);
                    pauseHi = 0;
                }

                return writeToFile(mBasePath + kStartIdxNode, mIdx * kRampSteps) &&
                       writeToFile(mBasePath + kDutyPctsNode, getScaledDutyPercent(value)) &&
                       writeToFile(mBasePath + kPauseLoNode, pauseLo) &&
                       writeToFile(mBasePath + kPauseHiNode, pauseHi) &&
                       writeToFile(mBasePath + kRampStepMsNode, stepDuration) &&
                       writeToFile(mBasePath + kBlinkNode, 1);
            }

            // Fallthrough to breath mode if timed is not supported
            FALLTHROUGH_INTENDED;
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

void LedDevice::setIdx(int idx) {
    mIdx = idx;
}

void LedDevice::dump(int fd) const {
    dprintf(fd, "Name: %s", mName.c_str());
    dprintf(fd, ", index: %d", mIdx);
    dprintf(fd, ", exists: %d", exists());
    dprintf(fd, ", base path: %s", mBasePath.c_str());
    dprintf(fd, ", max brightness: %u", mMaxBrightness);
    dprintf(fd, ", supports breath: %d", supportsBreath());
    dprintf(fd, ", supports timed: %d", supportsTimed());
    dprintf(fd, ", breath node: %s", mBreathNode.c_str());
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
