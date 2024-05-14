/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SysfsPoller.h"

#define LOG_TAG "SysfsPoller"

#include <android-base/logging.h>
#include <fcntl.h>
#include <sys/epoll.h>

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


        int epoll = ::epoll_create1(EPOLL_CLOEXEC);
        if (epoll < 0) {
            LOG(ERROR) << "Failed to create epoll fd: " << strerror(errno);
            return;
        }

        struct epoll_event event = {
                .events = EPOLLIN | EPOLLET | EPOLLPRI | EPOLLERR,
                .data = {
                        .fd = fd,
                },
        };

        int rc = epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &event);
        if (rc < 0) {
            LOG(ERROR) << "Failed to add fd to epoll: " << strerror(errno);
            return;
        }

        struct epoll_event events[1];

        while (mRunning) {
            rc = epoll_wait(epoll, events, 1, -1);
            if (rc != 1) {
                LOG(ERROR) << "epoll_wait failed on " << mPath << ": " << rc;
                break;
            }

            // The client may not want updates anymore
            if (mRunning) {
                mCallback();
            }
        }

        ::close(epoll);

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
