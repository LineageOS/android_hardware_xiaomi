/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <hidl/LegacySupport.h>
#include <vendor/xiaomi/hardware/touchfeature/1.0/ITouchFeature.h>

using android::hardware::defaultPassthroughServiceImplementation;
using vendor::xiaomi::hardware::touchfeature::V1_0::ITouchFeature;

int main() {
    return defaultPassthroughServiceImplementation<ITouchFeature>();
}
