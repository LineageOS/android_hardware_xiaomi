/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "XiaomiSmartPen.h"

#define LOG_TAG "XiaomiSmartPen"

#include <android-base/logging.h>
#include "../Utils.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static const std::string kPenEnabledPath = "/sys/touchpanel/pen";

XiaomiSmartPen::XiaomiSmartPen() {
    mAccessoryInfo = {
            .id = "active_pen:internal:1",
            .type = AccessoryType::ACTIVE_PEN,
            .isPersistent = true,
            .displayName = "Xiaomi Smart Pen",
            .displayId = "",
            .supportsToggling = true,
            .supportsBatteryQuerying = false,
    };

    onEnabledChanged();
}

::ndk::ScopedAStatus XiaomiSmartPen::setEnabled(bool enabled) {
    if (!writeToFile(kPenEnabledPath, enabled ? "1" : "0")) {
        LOG(ERROR) << "Failed to write to " << kPenEnabledPath;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mEnabled = enabled;

    updateStatus();

    return ::ndk::ScopedAStatus::ok();
}

void XiaomiSmartPen::updateStatus() {
    mAccessoryStatus = mEnabled ? AccessoryStatus::ENABLED : AccessoryStatus::DISABLED;

    onAccessoryStatusUpdated();
}

void XiaomiSmartPen::onEnabledChanged() {
    std::lock_guard lock(mDataMutex);

    if (!readFromFile(kPenEnabledPath, mEnabled)) {
        LOG(ERROR) << "Failed to read from " << kPenEnabledPath;
        return;
    }

    updateStatus();
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
