/*
 * Copyright (C) 2021-2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Utils.h"

#include <android-base/file.h>
#include <unistd.h>

using ::android::base::ReadFileToString;
using ::android::base::WriteStringToFile;

namespace aidl {
namespace android {
namespace hardware {
namespace light {

bool fileWriteable(const std::string& file) {
    return !access(file.c_str(), W_OK);
}

bool readFromFile(const std::string& file, std::string& content) {
    return ReadFileToString(file, &content, true);
}

bool readFromFile(const std::string& file, uint32_t& content) {
    std::string content_str;
    if (readFromFile(file, content_str))
        content = std::stoi(content_str);
    else
        return false;
    return true;
}

bool writeToFile(const std::string& file, const std::string content) {
    return WriteStringToFile(content, file);
}

bool writeToFile(const std::string& file, const uint32_t content) {
    return writeToFile(file, std::to_string(content));
}

rgb::rgb(uint32_t color) {
    // Extract brightness from AARRGGBB.
    float alpha = ((color >> 24) & 0xFF) / 255.0f;

    // Retrieve each of the RGB colors
    red = ((color >> 16) & 0xFF) / 255.0f;
    green = ((color >> 8) & 0xFF) / 255.0f;
    blue = (color & 0xFF) / 255.0f;

    // Scale RGB colors if a brightness has been applied by the user
    if (alpha > 0.0f && alpha < 1.0f) {
        red = red * alpha;
        green = green * alpha;
        blue = blue * alpha;
    }
}

bool rgb::isLit() {
    return !!red || !!green || !!blue;
}

static constexpr float kRedWeight = 77 / 256.0f;
static constexpr float kGreenWeight = 150 / 256.0f;
static constexpr float kBlueWeight = 29 / 256.0f;

float rgb::toBrightness() {
    return kRedWeight * red + kGreenWeight * green + kBlueWeight * blue;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
