/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/BnAccessory.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

class Accessory : public BnAccessory {
  public:
    Accessory() = delete;
    Accessory(const Accessory&) = delete;
    Accessory(const AccessoryInfo& accessoryInfo, const AccessoryStatus& accessoryStatus, const BatteryInfo& batteryInfo);

    ::ndk::ScopedAStatus getAccessoryInfo(AccessoryInfo* _aidl_return) override;
    ::ndk::ScopedAStatus getAccessoryStatus(AccessoryStatus* _aidl_return) override;
    ::ndk::ScopedAStatus setCallback(const ::std::shared_ptr<IAccessoryCallback>& callback) override;
    ::ndk::ScopedAStatus setEnabled(bool enabled) override;
    ::ndk::ScopedAStatus getBatteryInfo(BatteryInfo* _aidl_return) override;

    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

//  private:
    AccessoryInfo mAccessoryInfo;
    AccessoryStatus mAccessoryStatus;
    BatteryInfo mBatteryInfo;

    ::std::shared_ptr<IAccessoryCallback> mCallback;
};

} // namespace accessories
} // namespace lineage
} // namespace vendor
} // namespace aidl
