/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <string>

using ::aidl::android::hardware::light::HwLightState;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class LED {
public:
    LED(std::string type);

    bool exists();
    bool setBreath(const HwLightState& state, uint32_t brightness);
    bool setBrightness(uint8_t value);
private:
    std::string mBasePath;
    uint32_t mMaxBrightness;
    bool mBreath;
};

} // namespace light
} // namespace hardware
} // namespace android
} // namespace aidl
