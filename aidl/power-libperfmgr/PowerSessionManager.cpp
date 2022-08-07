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

#define LOG_TAG "powerhal-libperfmgr"
#define ATRACE_TAG (ATRACE_TAG_POWER | ATRACE_TAG_HAL)

#include "PowerSessionManager.h"

#include <android-base/file.h>
#include <log/log.h>
#include <perfmgr/HintManager.h>
#include <processgroup/processgroup.h>
#include <sys/syscall.h>
#include <utils/Trace.h>

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

using ::android::perfmgr::AdpfConfig;
using ::android::perfmgr::HintManager;

namespace {
/* there is no glibc or bionic wrapper */
struct sched_attr {
    __u32 size;
    __u32 sched_policy;
    __u64 sched_flags;
    __s32 sched_nice;
    __u32 sched_priority;
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
    __u32 sched_util_min;
    __u32 sched_util_max;
};

static int sched_setattr(int pid, struct sched_attr *attr, unsigned int flags) {
    if (!HintManager::GetInstance()->GetAdpfProfile()->mUclampMinOn) {
        ALOGV("PowerSessionManager:%s: skip", __func__);
        return 0;
    }
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

static void set_uclamp_min(int tid, int min) {
    static constexpr int32_t kMaxUclampValue = 1024;
    min = std::max(0, min);
    min = std::min(min, kMaxUclampValue);

    sched_attr attr = {};
    attr.size = sizeof(attr);

    attr.sched_flags = (SCHED_FLAG_KEEP_ALL | SCHED_FLAG_UTIL_CLAMP_MIN);
    attr.sched_util_min = min;

    int ret = sched_setattr(tid, &attr, 0);
    if (ret) {
        ALOGW("sched_setattr failed for thread %d, err=%d", tid, errno);
    }
}
}  // namespace

void PowerSessionManager::updateHintMode(const std::string &mode, bool enabled) {
    ALOGV("PowerSessionManager::updateHintMode: mode: %s, enabled: %d", mode.c_str(), enabled);
    if (enabled && mode.compare(0, 8, "REFRESH_") == 0) {
        if (mode.compare("REFRESH_120FPS") == 0) {
            mDisplayRefreshRate = 120;
        } else if (mode.compare("REFRESH_90FPS") == 0) {
            mDisplayRefreshRate = 90;
        } else if (mode.compare("REFRESH_60FPS") == 0) {
            mDisplayRefreshRate = 60;
        }
    }
    if (HintManager::GetInstance()->GetAdpfProfile()) {
        HintManager::GetInstance()->SetAdpfProfile(mode);
    }
}

void PowerSessionManager::updateHintBoost(const std::string &boost, int32_t durationMs) {
    ATRACE_CALL();
    ALOGV("PowerSessionManager::updateHintBoost: boost: %s, durationMs: %d", boost.c_str(),
          durationMs);
    if (boost.compare("DISPLAY_UPDATE_IMMINENT") == 0) {
        PowerHintMonitor::getInstance()->getLooper()->sendMessage(mWakeupHandler, NULL);
    }
}

void PowerSessionManager::wakeSessions() {
    std::lock_guard<std::mutex> guard(mLock);
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    std::unordered_set<PowerHintSession *> wakeupList;
    const int wakeupBoostValue = static_cast<int>(adpfConfig->mUclampMinInit);
    for (auto &it : mTidSessionListMap) {
        int tid = it.first;
        int maxboost = -1;
        // Find the max boost value among all the sessions that include the same TID.
        for (PowerHintSession *s : it.second) {
            if (!s->isActive())
                continue;
            // all active sessions need to be awakened.
            wakeupList.insert(s);
            if (s->isTimeout()) {
                maxboost = std::max(maxboost, s->getUclampMin());
            }
        }
        // Found the max boost and actally set to the task.
        if (maxboost != -1) {
            set_uclamp_min(tid, std::max(maxboost, wakeupBoostValue));
        }
    }
    for (PowerHintSession *s : wakeupList) {
        s->wakeup();
    }
}

int PowerSessionManager::getDisplayRefreshRate() {
    return mDisplayRefreshRate;
}

void PowerSessionManager::addPowerSession(PowerHintSession *session) {
    std::lock_guard<std::mutex> guard(mLock);
    for (auto t : session->getTidList()) {
        mTidSessionListMap[t].insert(session);
        if (mTidRefCountMap.find(t) == mTidRefCountMap.end()) {
            if (!SetTaskProfiles(t, {"ResetUclampGrp"})) {
                ALOGW("Failed to set ResetUclampGrp task profile for tid:%d", t);
            } else {
                mTidRefCountMap[t] = 1;
            }
            continue;
        }
        if (mTidRefCountMap[t] <= 0) {
            ALOGE("Error! Unexpected zero/negative RefCount:%d for tid:%d", mTidRefCountMap[t], t);
            continue;
        }
        mTidRefCountMap[t]++;
    }
    mSessions.insert(session);
}

void PowerSessionManager::removePowerSession(PowerHintSession *session) {
    std::lock_guard<std::mutex> guard(mLock);
    for (auto t : session->getTidList()) {
        if (mTidRefCountMap.find(t) == mTidRefCountMap.end()) {
            ALOGE("Unexpected Error! Failed to look up tid:%d in TidRefCountMap", t);
            continue;
        }
        mTidSessionListMap[t].erase(session);
        mTidRefCountMap[t]--;
        if (mTidRefCountMap[t] <= 0) {
            if (!SetTaskProfiles(t, {"NoResetUclampGrp"})) {
                ALOGW("Failed to set NoResetUclampGrp task profile for tid:%d", t);
            }
            mTidRefCountMap.erase(t);
        }
    }
    mSessions.erase(session);
}

void PowerSessionManager::setUclampMin(PowerHintSession *session, int val) {
    std::lock_guard<std::mutex> guard(mLock);
    setUclampMinLocked(session, val);
}

void PowerSessionManager::setUclampMinLocked(PowerHintSession *session, int val) {
    for (auto t : session->getTidList()) {
        // Get thex max uclamp.min across sessions which include the tid.
        int tidMax = 0;
        for (PowerHintSession *s : mTidSessionListMap[t]) {
            if (!s->isActive() || s->isTimeout())
                continue;
            tidMax = std::max(tidMax, s->getUclampMin());
        }
        set_uclamp_min(t, std::max(val, tidMax));
    }
}

std::optional<bool> PowerSessionManager::isAnyAppSessionActive() {
    std::lock_guard<std::mutex> guard(mLock);
    bool active = false;
    for (PowerHintSession *s : mSessions) {
        // session active and not stale is actually active.
        if (s->isActive() && !s->isTimeout() && s->isAppSession()) {
            active = true;
            break;
        }
    }
    if (active == mActive) {
        return std::nullopt;
    } else {
        mActive = active;
    }

    return active;
}

void PowerSessionManager::handleMessage(const Message &) {
    auto active = isAnyAppSessionActive();
    if (!active.has_value()) {
        return;
    }
    if (active.value()) {
        disableSystemTopAppBoost();
    } else {
        enableSystemTopAppBoost();
    }
}

void PowerSessionManager::WakeupHandler::handleMessage(const Message &) {
    PowerSessionManager::getInstance()->wakeSessions();
}

void PowerSessionManager::dumpToFd(int fd) {
    std::ostringstream dump_buf;
    std::lock_guard<std::mutex> guard(mLock);
    dump_buf << "========== Begin PowerSessionManager ADPF list ==========\n";
    for (PowerHintSession *s : mSessions) {
        s->dumpToStream(dump_buf);
        dump_buf << " Tid:Ref[";
        for (size_t i = 0, len = s->getTidList().size(); i < len; i++) {
            int t = s->getTidList()[i];
            dump_buf << t << ":" << mTidSessionListMap[t].size();
            if (i < len - 1) {
                dump_buf << ", ";
            }
        }
        dump_buf << "]\n";
    }
    dump_buf << "========== End PowerSessionManager ADPF list ==========\n";
    if (!::android::base::WriteStringToFd(dump_buf.str(), fd)) {
        ALOGE("Failed to dump one of session list to fd:%d", fd);
    }
}

void PowerSessionManager::enableSystemTopAppBoost() {
    if (HintManager::GetInstance()->IsHintSupported(kDisableBoostHintName)) {
        ALOGV("PowerSessionManager::enableSystemTopAppBoost!!");
        HintManager::GetInstance()->EndHint(kDisableBoostHintName);
    }
}

void PowerSessionManager::disableSystemTopAppBoost() {
    if (HintManager::GetInstance()->IsHintSupported(kDisableBoostHintName)) {
        ALOGV("PowerSessionManager::disableSystemTopAppBoost!!");
        HintManager::GetInstance()->DoHint(kDisableBoostHintName);
    }
}

// =========== PowerHintMonitor implementation start from here ===========
void PowerHintMonitor::start() {
    if (!isRunning()) {
        run("PowerHintMonitor", ::android::PRIORITY_HIGHEST);
    }
}

bool PowerHintMonitor::threadLoop() {
    while (true) {
        mLooper->pollOnce(-1);
    }
    return true;
}

sp<Looper> PowerHintMonitor::getLooper() {
    return mLooper;
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
