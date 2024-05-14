/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "XiaomiAccessories.h"

#define LOG_TAG "XiaomiAccessories"

#include <android-base/logging.h>
#include "mipad5keyboard/MiPad5Keyboard.h"
#include "xiaomismartpen/XiaomiSmartPen.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

XiaomiAccessories::XiaomiAccessories() {
    addAccessory(::ndk::SharedRefBase::make<MiPad5Keyboard>());
    addAccessory(::ndk::SharedRefBase::make<XiaomiSmartPen>());
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
