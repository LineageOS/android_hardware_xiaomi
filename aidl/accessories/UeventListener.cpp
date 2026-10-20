/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "UeventListener.h"

#define LOG_TAG "UeventListener"

#include <android-base/logging.h>
#include <cutils/uevent.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static constexpr int kUeventBufferSize = 64 * 1024;
static constexpr size_t kUeventMessageSize = 4096;

UeventListener::UeventListener(const std::string& match,
                               std::function<void(const Environment&)> callback)
    : mMatch(match),
      mCallback(callback),
      mStopFd(::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
      mRunning(false) {}

UeventListener::~UeventListener() {
    stop();

    ::close(mStopFd);
}

void UeventListener::start() {
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
        int fd = uevent_open_socket(kUeventBufferSize, true);
        if (fd < 0) {
            LOG(ERROR) << "Failed to open uevent socket: " << strerror(errno);
            mRunning = false;
            return;
        }

        struct pollfd fds[] = {
                {.fd = fd, .events = POLLIN},
                {.fd = mStopFd, .events = POLLIN},
        };
        char buffer[kUeventMessageSize];

        while (mRunning) {
            int rc = ::poll(fds, 2, -1);
            if (rc < 0) {
                if (errno == EINTR) {
                    continue;
                }
                LOG(ERROR) << "poll failed: " << strerror(errno);
                break;
            }

            if (!(fds[0].revents & POLLIN)) {
                continue;
            }

            ssize_t length = uevent_kernel_multicast_recv(fd, buffer, sizeof(buffer) - 1);
            if (length <= 0) {
                continue;
            }
            buffer[length] = '\0';

            // The message is a sequence of NUL terminated strings, the first being the header
            Environment environment;
            bool matches = false;
            for (const char* entry = buffer + strlen(buffer) + 1; entry < buffer + length;
                 entry += strlen(entry) + 1) {
                std::string pair(entry);
                if (pair == mMatch) {
                    matches = true;
                }

                size_t separator = pair.find('=');
                if (separator != std::string::npos) {
                    environment[pair.substr(0, separator)] = pair.substr(separator + 1);
                }
            }

            // The client may not want updates anymore
            if (matches && mRunning) {
                mCallback(environment);
            }
        }

        ::close(fd);
    });
}

void UeventListener::stop() {
    std::lock_guard lock(mMutex);

    mRunning = false;

    uint64_t value = 1;
    if (::write(mStopFd, &value, sizeof(value)) < 0) {
        LOG(ERROR) << "Failed to wake up the listener thread: " << strerror(errno);
    }

    if (mThread.joinable()) {
        mThread.join();
    }

    // Drain the event, so that the listener can be restarted
    (void)::read(mStopFd, &value, sizeof(value));
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
