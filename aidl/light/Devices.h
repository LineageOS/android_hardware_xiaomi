/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <vector>
#include "BacklightDevice.h"
#include "LedDevice.h"
#include "RgbLedDevice.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

// Backlight

std::vector<BacklightDevice> getBacklightDevices();
std::vector<LedDevice> getBacklightLedDevices();

// Buttons

std::vector<LedDevice> getButtonLedDevices();

// Notifications

RgbLedDevice getRgbLedDevice();
LedDevice getWhiteLedDevice();

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
