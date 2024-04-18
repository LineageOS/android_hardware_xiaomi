/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#include <android-base/logging.h>
#include "Button.h"
#include "Led.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

#define AutoHwLight(light) \
    { .id = (int32_t)light, .type = light, .ordinal = 0 }

Lights::Lights() {
    mBacklightDevices = getBacklightDevice();
    if (!mBacklightDevices.empty()) {
        mLights.push_back(AutoHwLight(LightType::BACKLIGHT));
    }

    mButtonDevices = getButtonDevices();
    if (!mButtonDevices.empty()) {
        mLights.push_back(AutoHwLight(LightType::BUTTONS));
    }

    mWhiteLED = LedDevice::kDefaultLeds[LedDevice::Type::WHITE].exists();

    mLights.push_back(AutoHwLight(LightType::BATTERY));
    mLights.push_back(AutoHwLight(LightType::NOTIFICATIONS));
    mLights.push_back(AutoHwLight(LightType::ATTENTION));
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    rgb color(state.color);

    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            if (mBacklightDevice) mBacklightDevice->setBacklight(color.toBrightness());
            break;
        case LightType::BUTTONS:
            for (auto& buttons : mButtonsPaths) writeToFile(buttons, color.isLit());
            break;
        case LightType::BATTERY:
        case LightType::NOTIFICATIONS:
        case LightType::ATTENTION:
            mLEDMutex.lock();

            if (type == LightType::BATTERY)
                mLastBatteryState = state;
            else if (type == LightType::NOTIFICATIONS)
                mLastNotificationState = state;
            else if (type == LightType::ATTENTION)
                mLastAttentionState = state;

            setLED();

            mLEDMutex.unlock();

            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* _aidl_return) {
    for (const auto& light : mLights) _aidl_return->push_back(light);

    return ndk::ScopedAStatus::ok();
}

void Lights::setLED() {
    bool rc = true;

    bool isBatteryLit = rgb(mLastBatteryState.color).isLit();
    bool isAttentionLit = rgb(mLastAttentionState.color).isLit();

    const HwLightState state = isBatteryLit     ? mLastBatteryState
                               : isAttentionLit ? mLastAttentionState
                                                : mLastNotificationState;

    rgb color(state.color);
    uint8_t blink = (state.flashOnMs != 0 && state.flashOffMs != 0);

    switch (state.flashMode) {
        case FlashMode::HARDWARE:
        case FlashMode::TIMED:
            if (mWhiteLED) {
                rc = kLEDs[WHITE].setBreath(blink);
            } else {
                rc = kLEDs[RED].setBreath(blink && color.red);
                rc &= kLEDs[GREEN].setBreath(blink && color.green);
                rc &= kLEDs[BLUE].setBreath(blink && color.blue);
            }
            if (rc) break;
            FALLTHROUGH_INTENDED;
        default:
            if (mWhiteLED) {
                rc = kLEDs[WHITE].setBrightness(color.toBrightness());
            } else {
                rc = kLEDs[RED].setBrightness(color.red);
                rc &= kLEDs[GREEN].setBrightness(color.green);
                rc &= kLEDs[BLUE].setBrightness(color.blue);
            }
            break;
    }

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
