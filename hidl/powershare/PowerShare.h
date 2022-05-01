/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VENDOR_LINEAGE_POWERSHARE_V1_0_POWERSHARE_H
#define VENDOR_LINEAGE_POWERSHARE_V1_0_POWERSHARE_H

#include <vendor/lineage/powershare/1.0/IPowerShare.h>

namespace vendor {
namespace lineage {
namespace powershare {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::hardware::Return;
using ::android::hardware::Void;

class PowerShare : public IPowerShare {
  public:
    Return<bool> isEnabled() override;
    Return<bool> setEnabled(bool enable) override;
    Return<uint32_t> getMinBattery() override;
    Return<uint32_t> setMinBattery(uint32_t minBattery) override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace powershare
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_POWERSHARE_V1_0_POWERSHARE_H
