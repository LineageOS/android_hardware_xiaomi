/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LED.h"

#include "Utils.h"

/*
 * 8 duty percent steps.
 */
#define RAMP_STEPS 8
/*
 * Each step will stay on for 50ms by default.
 */
#define RAMP_STEP_DURATION 50

namespace {
    /*
     * Each value represents a duty percent (0 - 100) for the led pwm.
     */
    static int32_t BRIGHTNESS_RAMP[RAMP_STEPS] = {0, 12, 25, 37, 50, 72, 85, 100};

    /*
     * Scale each value of the brightness ramp according to the
     * brightness of the color.
     */
    static std::string getScaledRamp(uint32_t brightness) {
        std::string ramp, pad;
        for (auto const& step : BRIGHTNESS_RAMP) {
            ramp += pad + std::to_string(step * brightness / 0xFF);
            pad = ",";
        }
        return ramp;
    }
} // anonymous namespace

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const uint32_t kDefaultMaxLedBrightness = 255;

LED::LED(std::string type) : mBasePath("/sys/class/leds/" + type + "/") {
    if (!readFromFile(mBasePath + "max_brightness", &mMaxBrightness))
        mMaxBrightness = kDefaultMaxLedBrightness;
    mBreath = fileWriteable(mBasePath + "breath");
}

bool LED::exists() {
    return fileWriteable(mBasePath + "brightness");
}

bool LED::setBreath(const HwLightState& state, uint32_t brightness) {
    uint8_t blink = (state.flashOnMs != 0 && state.flashOffMs != 0);
    if (!mBreath && blink) {
        /*
        * If the flashOnMs duration is not long enough to fit ramping up
        * and down at the default step duration, step duration is modified
        * to fit.
        */
        int32_t stepDuration = RAMP_STEP_DURATION;
        int32_t pauseHi = state.flashOnMs - (stepDuration * RAMP_STEPS * 2);
        int32_t pauseLo = state.flashOffMs;
        if (pauseHi < 0) {
            stepDuration = state.flashOnMs / (RAMP_STEPS * 2);
            pauseHi = 0;
        }
        writeToFile(mBasePath + "duty_pcts", getScaledRamp(brightness));
        writeToFile(mBasePath + "pause_lo", pauseLo);
        writeToFile(mBasePath + "pause_hi", pauseHi);
        writeToFile(mBasePath + "ramp_step_ms", stepDuration);
    }
    return writeToFile(mBasePath + (mBreath ? "breath" : "blink"), blink);
}

bool LED::setBrightness(uint8_t value) {
    return writeToFile(mBasePath + "brightness", value * mMaxBrightness / 0xFF);
}

} // namespace light
} // namespace hardware
} // namespace android
} // namespace aidl
