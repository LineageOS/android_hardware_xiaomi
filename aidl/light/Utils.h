/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

struct rgb {
    rgb(float r, float g, float b) : red(r), green(g), blue(b){};
    rgb(uint32_t color);
    rgb() : red(0.0f), green(0.0f), blue(0.0f){};

    float red;
    float green;
    float blue;

    bool isLit();
    float toBrightness();
};

bool fileWriteable(const std::string& file);
bool readFromFile(const std::string& file, std::string& content);
bool readFromFile(const std::string& file, uint32_t& content);
bool writeToFile(const std::string& file, uint32_t content);

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
