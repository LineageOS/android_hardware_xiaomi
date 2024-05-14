/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/accessories/BnAccessories.h>
#include <vector>
#include "Accessory.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

class Accessories : public BnAccessories {
  public:
    Accessories();

    ::ndk::ScopedAStatus getAccessories(::std::vector<::std::shared_ptr<IAccessory>>* _aidl_return) override;
    ::ndk::ScopedAStatus setCallback(const ::std::shared_ptr<IAccessoriesCallback>& callback) override;

    binder_status_t dump(int fd, const char** args, uint32_t numArgs) override;

  private:
    ::std::vector<::std::shared_ptr<Accessory>> mAccessories;
    ::std::shared_ptr<IAccessoriesCallback> mCallback;
};

} // namespace accessories
} // namespace lineage
} // namespace vendor
} // namespace aidl
