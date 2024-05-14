/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/Accessories.h>
#include <aidl/vendor/lineage/accessories/Accessory.h>
#include <aidl/vendor/lineage/accessories/BnAccessories.h>
#include <vector>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

class XiaomiAccessories : public Accessories {
  public:
    XiaomiAccessories();
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
