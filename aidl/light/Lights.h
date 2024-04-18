/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <mutex>
#include "Devices.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* _aidl_return) override;

    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

  private:
    std::vector<HwLight> mLights;

    Devices mDevices;

    HwLightState mLastBatteryState;
    HwLightState mLastNotificationsState;
    HwLightState mLastAttentionState;
    std::mutex mLedMutex;

    void updateNotificationColor();
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
