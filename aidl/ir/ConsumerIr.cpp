/*
 * SPDX-FileCopyrightText: 2017-2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "ConsumerIr"

#include "ConsumerIr.h"

#include <android-base/logging.h>
#include <fcntl.h>
#include <linux/lirc.h>
#include <string>

using std::vector;

namespace aidl {
namespace android {
namespace hardware {
namespace ir {

static const std::string kIrDevice = "/dev/lirc0";

static const int kDutyCycle = 33;

static vector<ConsumerIrFreqRange> kRangeVec{
        {.minHz = 30000, .maxHz = 60000},
};

::ndk::ScopedAStatus ConsumerIr::getCarrierFreqs(vector<ConsumerIrFreqRange>* _aidl_return) {
    *_aidl_return = kRangeVec;

    return ::ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus ConsumerIr::transmit(int32_t carrierFreqHz, const vector<int32_t>& pattern) {
    size_t entries = pattern.size();

    if (entries == 0) {
        return ::ndk::ScopedAStatus::ok();
    }

    int fd = open(kIrDevice.c_str(), O_RDWR);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open " << kIrDevice << ", error " << fd;

        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    int rc = ioctl(fd, LIRC_SET_SEND_CARRIER, &carrierFreqHz);
    if (rc < 0) {
        LOG(ERROR) << "Failed to set carrier " << carrierFreqHz << ", error: " << errno;

        close(fd);

        return ::ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    rc = ioctl(fd, LIRC_SET_SEND_DUTY_CYCLE, &kDutyCycle);
    if (rc < 0) {
        LOG(ERROR) << "Failed to set duty cycle " << kDutyCycle << ", error: " << errno;

        close(fd);

        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    if ((entries & 1) != 0) {
        rc = write(fd, pattern.data(), entries * sizeof(int32_t));
    } else {
        rc = write(fd, pattern.data(), (entries - 1) * sizeof(int32_t));
        usleep(pattern[entries - 1]);
    }

    if (rc < 0) {
        LOG(ERROR) << "Failed to write pattern, " << entries << " entries, error: " << errno;

        close(fd);

        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    close(fd);

    return ::ndk::ScopedAStatus::ok();
}

}  // namespace ir
}  // namespace hardware
}  // namespace android
}  // namespace aidl
