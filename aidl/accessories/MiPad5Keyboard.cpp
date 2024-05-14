/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MiPad5Keyboard.h"

#define LOG_TAG "MiPad5Keyboard"

#include <android-base/file.h>
#include <android-base/logging.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static const std::string kKeyboardEnabledPath = "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_enabled";
static const std::string kKeyboardConnectedPath = "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_conn_status";

MiPad5Keyboard::MiPad5Keyboard() {
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
}

::ndk::ScopedAStatus MiPad5Keyboard::setEnabled(bool enabled) {
    std::string path("/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_enabled");

    if (!::android::base::WriteStringToFile(enabled ? "1" : "0", path)) {
        LOG(ERROR) << "Failed to write to " << path;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mAccessoryStatus = enabled ? AccessoryStatus::ENABLED : AccessoryStatus::DISABLED;

    onAccessoryStatusUpdated();

    return ::ndk::ScopedAStatus::ok();
}

void MiPad5Keyboard::updateStatus() {
    std::string enabled;
    if (!::android::base::ReadFileToString(kKeyboardEnabledPath, &enabled)) {
        LOG(ERROR) << "Failed to read from " << kKeyboardEnabledPath;
        return;
    }

    std::string connected;
    if (!::android::base::ReadFileToString(kKeyboardConnectedPath, &connected)) {
        LOG(ERROR) << "Failed to read from " << kKeyboardConnectedPath;
        return;
    }

    mAccessoryStatus = enabled == "1" ? AccessoryStatus::ENABLED : AccessoryStatus::DISABLED;

    onAccessoryStatusUpdated();
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
