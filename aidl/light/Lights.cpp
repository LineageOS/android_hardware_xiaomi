/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Lights.h"

#define LOG_TAG "Lights"

#include <android-base/logging.h>
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

#define AutoHwLight(light) \
    { .id = static_cast<int32_t>(light), .ordinal = 0, .type = light }

Lights::Lights() {
    if (mDevices.hasBacklightDevices()) {
        mLights.push_back(AutoHwLight(LightType::BACKLIGHT));
    }

    if (mDevices.hasButtonDevices()) {
        mLights.push_back(AutoHwLight(LightType::BUTTONS));
    }

    if (mDevices.hasNotificationDevices()) {
        mLights.push_back(AutoHwLight(LightType::BATTERY));
        mLights.push_back(AutoHwLight(LightType::NOTIFICATIONS));
        mLights.push_back(AutoHwLight(LightType::ATTENTION));
    }
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    rgb color(state.color);

    LightType type = static_cast<LightType>(id);
    switch (type) {
        case LightType::BACKLIGHT:
            mDevices.setBacklightColor(color);
            break;
        case LightType::BUTTONS:
            mDevices.setButtonsColor(color);
            break;
        case LightType::BATTERY:
            mLastBatteryState = state;
            updateNotificationColor();
            break;
        case LightType::NOTIFICATIONS:
            mLastNotificationsState = state;
            updateNotificationColor();
            break;
        case LightType::ATTENTION:
            mLastAttentionState = state;
            updateNotificationColor();
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

binder_status_t Lights::dump(int fd, const char** /*args*/, uint32_t /*numArgs*/) {
    dprintf(fd, "Lights AIDL:\n");
    dprintf(fd, "\n");

    dprintf(fd, "Lights:\n");
    for (const auto& light : mLights) {
        dprintf(fd, "- %d: LightType::%s\n", light.id, toString(light.type).c_str());
    }
    dprintf(fd, "\n");

    dprintf(fd, "Devices:\n");
    mDevices.dump(fd);
    dprintf(fd, "\n");

    return STATUS_OK;
}

void Lights::updateNotificationColor() {
    std::lock_guard<std::mutex> lock(mLedMutex);

    bool isBatteryLit = rgb(mLastBatteryState.color).isLit();
    bool isAttentionLit = rgb(mLastAttentionState.color).isLit();

    const HwLightState state = isBatteryLit     ? mLastBatteryState
                               : isAttentionLit ? mLastAttentionState
                                                : mLastNotificationsState;

    rgb color(state.color);

    LightMode lightMode;
    switch (state.flashMode) {
        case FlashMode::NONE:
            lightMode = LightMode::STATIC;
            break;
        case FlashMode::TIMED:
        case FlashMode::HARDWARE:
            lightMode = LightMode::BREATH;
            break;
        default:
            LOG(ERROR) << "Unknown flash mode: " << static_cast<int>(state.flashMode);
            lightMode = LightMode::STATIC;
            break;
    }

    mDevices.setNotificationColor(color, lightMode);

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
