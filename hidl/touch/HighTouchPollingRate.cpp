/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "HighTouchPollingRateService"

#include "HighTouchPollingRate.h"

#include <fstream>

namespace vendor {
namespace lineage {
namespace touch {
namespace V1_0 {
namespace implementation {

Return<bool> HighTouchPollingRate::isEnabled() {
    std::ifstream file(HIGH_TOUCH_POLLING_PATH);
    int enabled;
    file >> enabled;

    return enabled == 1;
}

Return<bool> HighTouchPollingRate::setEnabled(bool enabled) {
    std::ofstream file(HIGH_TOUCH_POLLING_PATH);
    file << (enabled ? "1" : "0");
    return !file.fail();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace touch
}  // namespace lineage
}  // namespace vendor
