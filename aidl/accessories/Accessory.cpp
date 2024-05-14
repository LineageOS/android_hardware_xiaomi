/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Accessory.h"

#define LOG_TAG "Accessory"

#include <android-base/logging.h>

using std::optional;
using std::shared_ptr;

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

Accessory::Accessory(const AccessoryInfo& accessoryInfo, const AccessoryStatus& accessoryStatus,
                     const BatteryInfo& batteryInfo) : mAccessoryInfo(accessoryInfo),
                     mAccessoryStatus(accessoryStatus), mBatteryInfo(batteryInfo) {
    LOG(INFO) << "Accessory()";
}

::ndk::ScopedAStatus Accessory::getAccessoryInfo(AccessoryInfo* _aidl_return) {
    LOG(INFO) << "getAccessoryInfo()";

    *_aidl_return = mAccessoryInfo;

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Accessory::getAccessoryStatus(AccessoryStatus* _aidl_return) {
    LOG(INFO) << "getAccessoryStatus()";

    *_aidl_return = mAccessoryStatus;

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Accessory::setCallback(const shared_ptr<IAccessoryCallback>& callback) {
    LOG(INFO) << "setCallback()";

    mCallback = callback;

    // Notify the callback of all the accessory info
    if (mCallback != nullptr) {
        LOG(INFO) << "Notifying callback of accessory info";
        mCallback->onAccessoryInfoUpdated(mAccessoryInfo);
        mCallback->onAccessoryStatusUpdated(mAccessoryStatus);
        mCallback->onBatteryInfoUpdated(mBatteryInfo);
    } else {
        LOG(INFO) << "No callback to notify";
    }

    return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

::ndk::ScopedAStatus Accessory::setEnabled(bool enabled) {
    LOG(INFO) << "setEnabled(" << enabled << ")";

    mAccessoryStatus = enabled ? AccessoryStatus::ENABLED : AccessoryStatus::DISABLED;

    if (mCallback != nullptr) {
        LOG(INFO) << "Notifying callback of status change";
        mCallback->onAccessoryStatusUpdated(mAccessoryStatus);
    } else {
        LOG(INFO) << "No callback to notify";
    }

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Accessory::getBatteryInfo(BatteryInfo* _aidl_return) {
    LOG(INFO) << "getBatteryInfo()";

    *_aidl_return = mBatteryInfo;

    return ::ndk::ScopedAStatus::ok();
}

binder_status_t Accessory::dump(int fd, const char** /*args*/, uint32_t /*numArgs*/) {
    dprintf(fd, "Accessory:\n");
    dprintf(fd, "  id: %s\n", mAccessoryInfo.id.c_str());
    dprintf(fd, "  type: %d\n", static_cast<int>(mAccessoryInfo.type));
    dprintf(fd, "  isPersistent: %d\n", mAccessoryInfo.isPersistent);
    dprintf(fd, "  displayName: %s\n", mAccessoryInfo.displayName.c_str());
    dprintf(fd, "  displayId: %s\n", mAccessoryInfo.displayId.c_str());
    dprintf(fd, "  supportsToggling: %d\n", mAccessoryInfo.supportsToggling);
    dprintf(fd, "  supportsBatteryQuerying: %d\n", mAccessoryInfo.supportsBatteryQuerying);
    dprintf(fd, "  status: %d\n", static_cast<int>(mAccessoryStatus));
    dprintf(fd, "  battery status: %d\n", static_cast<int>(mBatteryInfo.status));
    dprintf(fd, "  battery level: %d\n", mBatteryInfo.levelPercentage);

    return STATUS_OK;
}

} // namespace accessories
} // namespace lineage
} // namespace vendor
} // namespace aidl
