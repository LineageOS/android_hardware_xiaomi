/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "PowerShareService"

#include "PowerShare.h"
#include <hidl/HidlTransportSupport.h>
#include <fstream>

namespace vendor {
namespace lineage {
namespace powershare {
namespace V1_0 {
namespace implementation {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

Return<bool> PowerShare::isEnabled() {
    const auto value = get<std::string>(WIRELESS_TX_ENABLE_PATH, "0");
    return !(value == "disable" || value == "0");
}

Return<bool> PowerShare::setEnabled(bool enable) {
    set(WIRELESS_TX_ENABLE_PATH, enable ? 1 : 0);

    return isEnabled();
}

Return<uint32_t> PowerShare::getMinBattery() {
    return 0;
}

Return<uint32_t> PowerShare::setMinBattery(uint32_t) {
    return getMinBattery();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace powershare
}  // namespace lineage
}  // namespace vendor
