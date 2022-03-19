/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TouchFeature.h"

#define TOUCH_DEV_PATH "/dev/xiaomi-touch"
#define TOUCH_MAGIC 0x5400

enum MODE_CMD {
	SET_CUR_VALUE = TOUCH_MAGIC,
	GET_CUR_VALUE,
	GET_DEF_VALUE,
	GET_MIN_VALUE,
	GET_MAX_VALUE,
	GET_MODE_VALUE,
	RESET_MODE,
	SET_LONG_VALUE,
};

namespace vendor::xiaomi::hardware::touchfeature::implementation {

TouchFeature::TouchFeature() {
    mTouchFd = android::base::unique_fd(open(TOUCH_DEV_PATH, O_RDWR));
}

Return<int32_t> TouchFeature::setTouchMode(int32_t mode, int32_t value) {
    int32_t data[2] = {mode, value};
    ioctl(mTouchFd.get(), SET_CUR_VALUE, &data);
    return 0;
}

Return<int32_t> TouchFeature::getTouchModeCurValue(int32_t mode) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), GET_CUR_VALUE, &data);
    return data[0];
}

Return<int32_t> TouchFeature::getTouchModeMaxValue(int32_t mode) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), GET_MAX_VALUE, &data);
    return data[0];
}

Return<int32_t> TouchFeature::getTouchModeMinValue(int32_t mode) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), GET_MIN_VALUE, &data);
    return data[0];
}

Return<int32_t> TouchFeature::getTouchModeDefValue(int32_t mode) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), GET_DEF_VALUE, &data);
    return data[0];
}

Return<int32_t> TouchFeature::resetTouchMode(int32_t mode) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), RESET_MODE, &data);
    return 0;
}

Return<void> TouchFeature::getModeValues(int32_t mode, getModeValues_cb _hidl_cb) {
    int32_t data[1] = {mode};
    ioctl(mTouchFd.get(), GET_MODE_VALUE, &data);
    _hidl_cb(data[0]);
    return Void();
}

ITouchFeature* HIDL_FETCH_ITouchFeature(const char* /* name */) {
    return new TouchFeature();
}

} // namespace vendor::xiaomi::hardware::touchfeature::implementation
