/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SysfsPoller.h"

#define LOG_TAG "SysfsPoller"

#include <android-base/logging.h>
#include <fcntl.h>
#include <poll.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

SysfsPoller::SysfsPoller(const std::string& path, std::function<void()> callback)
    : mPath(path), mCallback(callback), mRunning(false) {}

SysfsPoller::~SysfsPoller() {
    stop();
}

void SysfsPoller::start() {
    std::lock_guard lock(mMutex);

    if (mRunning) {
        return;
    }

    // Clear the current thread regardless of its state
    if (mThread.joinable()) {
        mThread.join();
    }

    mRunning = true;

    mThread = std::thread([this]() {
        int fd = ::open(mPath.c_str(), O_RDONLY);
        if (fd < 0) {
            LOG(ERROR) << "Failed to open " << mPath << ": " << strerror(errno);
            mRunning = false;
            return;
        }

        struct pollfd pollRequest = {
                .fd = fd,
                .events = POLLERR | POLLPRI,
                .revents = 0,
        };

        while (mRunning) {
            int rc = poll(&pollRequest, 1, -1);
            if (rc < 0) {
                LOG(ERROR) << "Poll failed on " << mPath << ": " << strerror(errno);
                continue;
            }

            if (mRunning) {
                // The client doesn't want updates anymore
                mCallback();
            }
        }

        ::close(fd);

        return;
    });
}

void SysfsPoller::stop() {
    std::lock_guard lock(mMutex);

    mRunning = false;

    if (mThread.joinable()) {
        mThread.join();
    }
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
