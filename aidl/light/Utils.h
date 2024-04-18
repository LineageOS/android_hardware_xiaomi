/*
 * Copyright (C) 2021-2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

struct rgb {
    rgb();
    rgb(uint8_t r, uint8_t g, uint8_t b);
    rgb(uint32_t color);

    uint8_t red;
    uint8_t green;
    uint8_t blue;

    bool isLit();
    uint8_t toBrightness();
};

uint32_t scaleBrightness(uint8_t brightness, uint32_t maxBrightness);

template <typename T>
bool readFromFile(const std::string& file, T& content) {
    std::ifstream fileStream(file);

    if (!fileStream) {
        return false;
    }

    fileStream >> content;
    return true;
}

template <typename T>
bool writeToFile(const std::string& file, const T content) {
    std::ofstream fileStream(file);

    if (!fileStream) {
        return false;
    }

    fileStream << content;
    return true;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
