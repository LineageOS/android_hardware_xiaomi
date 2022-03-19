/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <android-base/unique_fd.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/xiaomi/hardware/touchfeature/1.0/ITouchFeature.h>

using vendor::xiaomi::hardware::touchfeature::V1_0::ITouchFeature;

namespace vendor::xiaomi::hardware::touchfeature::implementation {

using ::android::base::unique_fd;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct TouchFeature : public ITouchFeature {
public:
    TouchFeature();

    Return<int32_t> setTouchMode(int32_t mode, int32_t value) override;
    Return<int32_t> getTouchModeCurValue(int32_t mode) override;
    Return<int32_t> getTouchModeMaxValue(int32_t mode) override;
    Return<int32_t> getTouchModeMinValue(int32_t mode) override;
    Return<int32_t> getTouchModeDefValue(int32_t mode) override;
    Return<int32_t> resetTouchMode(int32_t mode) override;
    Return<void> getModeValues(int32_t mode, getModeValues_cb _hidl_cb) override;

private:
    unique_fd mTouchFd;
};

extern "C" ITouchFeature* HIDL_FETCH_ITouchFeature(const char* name);

} // namespace vendor::xiaomi::hardware::touchfeature::implementation
