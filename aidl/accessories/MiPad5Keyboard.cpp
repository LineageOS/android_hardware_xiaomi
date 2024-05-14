/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MiPad5Keyboard.h"

#define LOG_TAG "MiPad5Keyboard"

#include <android-base/logging.h>
#include "Utils.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static const std::string kKeyboardEnabledPath =
        "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_enabled";
static const std::string kKeyboardConnectedPath =
        "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_conn_status";

MiPad5Keyboard::MiPad5Keyboard()
    : mConnectedPoller(kKeyboardConnectedPath, [this]() { onConnectedChanged(); }),
      mEnabledPoller(kKeyboardEnabledPath, [this]() { onEnabledChanged(); }) {
    mAccessoryInfo = {
            .id = "keyboard:internal:1",
            .type = AccessoryType::KEYBOARD,
            .isPersistent = true,
            .displayName = "Xiaomi Keyboard",
            .displayId = "",
            .supportsToggling = true,
            .supportsBatteryQuerying = false,
    };
    mAccessoryStatus = AccessoryStatus::ENABLED;

    mConnectedPoller.start();
    mEnabledPoller.start();
}

::ndk::ScopedAStatus MiPad5Keyboard::setEnabled(bool enabled) {
    if (!writeToFile(kKeyboardEnabledPath, enabled ? "1" : "0")) {
        LOG(ERROR) << "Failed to write to " << kKeyboardEnabledPath;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mEnabled = enabled;

    updateStatus();

    return ::ndk::ScopedAStatus::ok();
}

void MiPad5Keyboard::updateStatus() {
    mAccessoryStatus = !mConnected ? AccessoryStatus::DISCONNECTED
                       : mEnabled  ? AccessoryStatus::ENABLED
                                   : AccessoryStatus::DISABLED;

    onAccessoryStatusUpdated();
}

void MiPad5Keyboard::onConnectedChanged() {
    std::lock_guard lock(mDataMutex);

    if (!readFromFile(kKeyboardConnectedPath, mConnected)) {
        LOG(ERROR) << "Failed to read from " << kKeyboardConnectedPath;
        return;
    }

    updateStatus();
}

void MiPad5Keyboard::onEnabledChanged() {
    std::lock_guard lock(mDataMutex);

    if (!readFromFile(kKeyboardEnabledPath, mEnabled)) {
        LOG(ERROR) << "Failed to read from " << kKeyboardEnabledPath;
        return;
    }

    updateStatus();
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
