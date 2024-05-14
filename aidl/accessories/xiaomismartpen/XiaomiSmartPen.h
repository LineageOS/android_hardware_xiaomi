/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/Accessory.h>
#include <aidl/vendor/lineage/accessories/BnAccessory.h>
#include <mutex>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

/**
 * Xiaomi Smart Pen accessory.
 */
class XiaomiSmartPen : public Accessory {
  public:
    XiaomiSmartPen();
    XiaomiSmartPen(const Accessory&) = delete;

    ::ndk::ScopedAStatus setEnabled(bool enabled) override;

  private:
    bool mEnabled;

    std::mutex mDataMutex;

    void onEnabledChanged();
    void updateStatus();
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
