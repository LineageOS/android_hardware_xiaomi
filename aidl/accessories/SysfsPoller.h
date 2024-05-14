/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <mutex>
#include <string>
#include <thread>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

/**
 * Wrapper for sysfs nodes using sysfs_notify().
 * Once a notification is received, the callback function is called.
 */
class SysfsPoller {
  public:
    SysfsPoller() = delete;
    SysfsPoller(const SysfsPoller&) = delete;
    SysfsPoller(const std::string& path, std::function<void()> callback);

    ~SysfsPoller();

    void start();
    void stop();

  private:
    std::string mPath;
    std::function<void()> mCallback;

    std::atomic<bool> mRunning;
    std::thread mThread;
    std::mutex mMutex;
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
