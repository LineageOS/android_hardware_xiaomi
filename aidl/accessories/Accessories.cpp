/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Accessories.h"

#define LOG_TAG "Accessories"

#include <android-base/logging.h>

using std::optional;
using std::shared_ptr;
using std::vector;

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

Accessories::Accessories() {
    LOG(INFO) << "Accessories()";

    mAccessories.push_back(::ndk::SharedRefBase::make<Accessory>(
        (AccessoryInfo) {
            .id = "keyboard:dummy:1",
            .type = AccessoryType::KEYBOARD,
            .isPersistent = true,
            .displayName = "Dummy Keyboard",
            .displayId = "123456",
            .supportsToggling = true,
            .supportsBatteryQuerying = true,
        },
        AccessoryStatus::ENABLED,
        (BatteryInfo) {
            .status = BatteryStatus::NOT_CHARGING,
            .levelPercentage = 100,
        }
    ));

    mAccessories.push_back(::ndk::SharedRefBase::make<Accessory>(
        (AccessoryInfo) {
            .id = "pen:dummy:1",
            .type = AccessoryType::ACTIVE_PEN,
            .isPersistent = true,
            .displayName = "Dummy Active Pen",
            .displayId = "123456",
            .supportsToggling = true,
            .supportsBatteryQuerying = true,
        },
        AccessoryStatus::ENABLED,
        (BatteryInfo) {
            .status = BatteryStatus::NOT_CHARGING,
            .levelPercentage = 100,
        }
    ));
}

::ndk::ScopedAStatus Accessories::getAccessories(vector<shared_ptr<IAccessory>>* _aidl_return) {
    LOG(INFO) << "getAccessories()";

    for (const auto& accessory : mAccessories) {
        LOG(INFO) << "Adding accessory " << accessory->mAccessoryInfo.id;
        _aidl_return->push_back(accessory);
    }

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Accessories::setCallback(const shared_ptr<IAccessoriesCallback>& callback) {
    LOG(INFO) << "setCallback()";
    mCallback = callback;

    // Notify the callback of the new accessories
    if (mCallback != nullptr) {
        LOG(INFO) << "Notifying callback of all accessories";
        for (const auto& accessory : mAccessories) {
            LOG(INFO) << "Notifying callback of accessory " << accessory->mAccessoryInfo.id;
            mCallback->onAccessoryAdded(accessory);
        }
    } else {
        LOG(INFO) << "No callback to notify";
    }

    return ::ndk::ScopedAStatus::ok();
}

binder_status_t Accessories::dump(int fd, const char** args, uint32_t numArgs) {
    dprintf(fd, "Accessories:\n");
    for (const auto& accessory : mAccessories) {
        if (accessory == nullptr) {
            continue;
        }

        accessory->dump(fd, args, numArgs);
    }

    return STATUS_OK;
}

} // namespace accessories
} // namespace lineage
} // namespace vendor
} // namespace aidl
