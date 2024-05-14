/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <fstream>
#include <string>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

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

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
