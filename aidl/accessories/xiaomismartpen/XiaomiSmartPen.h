/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/Accessory.h>
#include <aidl/vendor/lineage/accessories/BnAccessory.h>
#include <mutex>
#include <string>
#include "../UeventListener.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

/**
 * Xiaomi Smart Pen accessory.
 * The pen is charged by the wireless charger once attached to the device, which also reports
 * its Bluetooth address and battery level.
 */
class XiaomiSmartPen : public Accessory {
  public:
    XiaomiSmartPen();
    XiaomiSmartPen(const Accessory&) = delete;

    ::ndk::ScopedAStatus setEnabled(bool enabled) override;

  private:
    bool mEnabled = true;

    int mBatteryLevel = BatteryInfo::BATTERY_LEVEL_UNKNOWN;
    UeventListener mWirelessListener;

    std::mutex mDataMutex;

    void onEnabledChanged();
    void onWirelessChanged(const std::string& chargeState, const std::string& mac,
                           const std::string& soc);
    void updateStatus();
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
