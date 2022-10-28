/*
 * Copyright 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <aidl/android/hardware/power/BnPowerHintSession.h>
#include <aidl/android/hardware/power/WorkDuration.h>
#include <utils/Looper.h>
#include <utils/Thread.h>

#include <mutex>
#include <unordered_map>

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

using aidl::android::hardware::power::BnPowerHintSession;
using aidl::android::hardware::power::WorkDuration;
using ::android::Message;
using ::android::MessageHandler;
using ::android::sp;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::chrono::time_point;

struct AppHintDesc {
    AppHintDesc(int32_t tgid, int32_t uid, std::vector<int> threadIds)
        : tgid(tgid),
          uid(uid),
          threadIds(std::move(threadIds)),
          duration(0LL),
          current_min(0),
          is_active(true),
          update_count(0),
          integral_error(0),
          previous_error(0) {}
    std::string toString() const;
    const int32_t tgid;
    const int32_t uid;
    const std::vector<int> threadIds;
    nanoseconds duration;
    int current_min;
    // status
    std::atomic<bool> is_active;
    // pid
    uint64_t update_count;
    int64_t integral_error;
    int64_t previous_error;
};

class PowerHintSession : public BnPowerHintSession {
  public:
    explicit PowerHintSession(int32_t tgid, int32_t uid, const std::vector<int32_t> &threadIds,
                              int64_t durationNanos);
    ~PowerHintSession();
    ndk::ScopedAStatus close() override;
    ndk::ScopedAStatus pause() override;
    ndk::ScopedAStatus resume() override;
    ndk::ScopedAStatus updateTargetWorkDuration(int64_t targetDurationNanos) override;
    ndk::ScopedAStatus reportActualWorkDuration(
            const std::vector<WorkDuration> &actualDurations) override;
    bool isActive();
    bool isTimeout();
    void wakeup();
    void setStale();
    // Is this hint session for a user application
    bool isAppSession();
    const std::vector<int> &getTidList() const;
    int getUclampMin();
    void dumpToStream(std::ostream &stream);

    time_point<steady_clock> getStaleTime();

  private:
    class StaleTimerHandler : public MessageHandler {
      public:
        StaleTimerHandler(PowerHintSession *session)
            : mSession(session), mIsMonitoring(false), mIsSessionDead(false) {}
        void updateTimer();
        void updateTimer(time_point<steady_clock> staleTime);
        void handleMessage(const Message &message) override;
        void setSessionDead();

      private:
        PowerHintSession *mSession;
        std::mutex mClosedLock;
        std::mutex mMessageLock;
        std::atomic<time_point<steady_clock>> mStaleTime;
        std::atomic<bool> mIsMonitoring;
        bool mIsSessionDead;
    };

  private:
    void updateUniveralBoostMode();
    int setSessionUclampMin(int32_t min);
    void tryToSendPowerHint(std::string hint);
    std::string getIdString() const;
    AppHintDesc *mDescriptor = nullptr;
    sp<StaleTimerHandler> mStaleTimerHandler;
    std::atomic<time_point<steady_clock>> mLastUpdatedTime;
    sp<MessageHandler> mPowerManagerHandler;
    std::mutex mSessionLock;
    std::atomic<bool> mSessionClosed = false;
    // To cache the status of whether ADPF hints are supported.
    std::unordered_map<std::string, std::optional<bool>> mSupportedHints;
};

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
