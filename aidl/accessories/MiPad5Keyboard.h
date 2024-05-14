/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/Accessory.h>
#include <aidl/vendor/lineage/accessories/BnAccessory.h>
#include <mutex>
#include "SysfsPoller.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

/**
 * Mi Pad 5 (nabu) keyboard accessory.
 */
class MiPad5Keyboard : public Accessory {
  public:
    MiPad5Keyboard();
    MiPad5Keyboard(const Accessory&) = delete;

    ::ndk::ScopedAStatus setEnabled(bool enabled) override;

  private:
    bool mConnected;
    SysfsPoller mConnectedPoller;

    bool mEnabled;
    SysfsPoller mEnabledPoller;

    std::mutex mDataMutex;

    void onConnectedChanged();
    void onEnabledChanged();
    void updateStatus();
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
