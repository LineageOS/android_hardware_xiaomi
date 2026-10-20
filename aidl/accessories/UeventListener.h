/*
 * SPDX-FileCopyrightText: 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

/**
 * Listens to kernel uevents containing the given KEY=VALUE pair.
 * Once a matching uevent is received, the callback function is called with its environment.
 */
class UeventListener {
  public:
    using Environment = std::map<std::string, std::string>;

    UeventListener() = delete;
    UeventListener(const UeventListener&) = delete;
    UeventListener(const std::string& match, std::function<void(const Environment&)> callback);

    ~UeventListener();

    void start();
    void stop();

  private:
    std::string mMatch;
    std::function<void(const Environment&)> mCallback;

    int mStopFd;

    std::atomic<bool> mRunning;
    std::thread mThread;
    std::mutex mMutex;
};

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
