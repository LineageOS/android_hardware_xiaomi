/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#define LOG_TAG "Lights"

#include <android-base/logging.h>
#include "Devices.h"
#include "LedDevice.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

#define AutoHwLight(light) \
    { .id = (int32_t)light, .type = light, .ordinal = 0 }

Lights::Lights() : mRgbLedDevice(getRgbLedDevice()), mWhiteLed(getWhiteLedDevice()) {
    mBacklightDevices = getBacklightDevices();
    mBacklightLedDevices = getBacklightLedDevices();
    if (!mBacklightDevices.empty() || !mBacklightLedDevices.empty()) {
        mLights.push_back(AutoHwLight(LightType::BACKLIGHT));
    } else {
        LOG(INFO) << "No backlight device found";
    }

    mButtonLedDevices = getButtonLedDevices();
    if (!mButtonLedDevices.empty()) {
        mLights.push_back(AutoHwLight(LightType::BUTTONS));
    } else {
        LOG(INFO) << "No button device found";
    }

    if (mRgbLedDevice.exists() || mWhiteLed.exists()) {
        mLights.push_back(AutoHwLight(LightType::BATTERY));
        mLights.push_back(AutoHwLight(LightType::NOTIFICATIONS));
        mLights.push_back(AutoHwLight(LightType::ATTENTION));
    } else {
        LOG(INFO) << "No notifications device found";
    }
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    rgb color(state.color);

    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            for (auto& device : mBacklightDevices) {
                device.setBrightness(color.toBrightness());
            }
            for (auto& device : mBacklightLedDevices) {
                device.setBrightness(color.toBrightness());
            }
            break;
        case LightType::BUTTONS:
            for (auto& device : mButtonLedDevices) {
                device.setBrightness(color.toBrightness());
            }
            break;
        case LightType::BATTERY:
            mLastBatteryState = state;
            updateLeds();
            break;
        case LightType::NOTIFICATIONS:
            mLastNotificationsState = state;
            updateLeds();
            break;
        case LightType::ATTENTION:
            mLastAttentionState = state;
            updateLeds();
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
            break;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* _aidl_return) {
    for (const auto& light : mLights) {
        _aidl_return->push_back(light);
    }

    return ndk::ScopedAStatus::ok();
}

void Lights::updateLeds() {
    std::lock_guard<std::mutex> lock(mLedMutex);

    bool rc = true;

    bool isBatteryLit = rgb(mLastBatteryState.color).isLit();
    bool isAttentionLit = rgb(mLastAttentionState.color).isLit();

    const HwLightState state = isBatteryLit     ? mLastBatteryState
                               : isAttentionLit ? mLastAttentionState
                                                : mLastNotificationsState;

    rgb color(state.color);
    float blink = (state.flashOnMs != 0 && state.flashOffMs != 0) ? 1.0f : 0.0f;

    switch (state.flashMode) {
        case FlashMode::TIMED:
        case FlashMode::HARDWARE:
            if (mWhiteLed.exists()) {
                rc &= mWhiteLed.setBrightness(color.toBrightness(), LightMode::BREATH);
            } else {
                rc &= mRgbLedDevice.setBrightness(color, LightMode::BREATH);
            }

            if (rc) {
                break;
            }
            // Fall back to normal mode if hardware timed mode is not supported
            rc = true;
            FALLTHROUGH_INTENDED;
        default:
            if (mWhiteLed.exists()) {
                rc &= mWhiteLed.setBrightness(color.toBrightness(), LightMode::STATIC);
            } else {
                rc &= mRgbLedDevice.setBrightness(color, LightMode::STATIC);
            }
            break;
    }

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
