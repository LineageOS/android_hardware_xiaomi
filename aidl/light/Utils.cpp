/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

rgb::rgb() : red(0), green(0), blue(0) {}

rgb::rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b){};

rgb::rgb(uint32_t color) {
    // Extract brightness from AARRGGBB.
    uint8_t alpha = (color >> 24) & 0xFF;

    // Retrieve each of the RGB colors
    red = (color >> 16) & 0xFF;
    green = (color >> 8) & 0xFF;
    blue = color & 0xFF;

    // Scale RGB colors if a brightness has been applied by the user
    if (alpha > 0 && alpha < 0xFF) {
        red = red * alpha / 0xFF;
        green = green * alpha / 0xFF;
        blue = blue * alpha / 0xFF;
    }
}

bool rgb::isLit() {
    return !!red || !!green || !!blue;
}

static constexpr uint8_t kRedWeight = 77;
static constexpr uint8_t kGreenWeight = 150;
static constexpr uint8_t kBlueWeight = 29;

uint8_t rgb::toBrightness() {
    return (kRedWeight * red + kGreenWeight * green + kBlueWeight * blue) >> 8;
}

uint32_t scaleBrightness(uint8_t brightness, uint32_t maxBrightness) {
    return brightness * maxBrightness / 0xFF;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
