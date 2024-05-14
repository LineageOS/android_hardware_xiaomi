/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "XiaomiAccessories.h"

#define LOG_TAG "XiaomiAccessories"

#include <android-base/logging.h>
#include "MiPad5Keyboard.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

XiaomiAccessories::XiaomiAccessories() {
    addAccessory(::ndk::SharedRefBase::make<MiPad5Keyboard>());
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
