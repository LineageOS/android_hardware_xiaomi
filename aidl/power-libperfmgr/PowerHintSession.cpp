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

#include "PowerHintSession.h"

#include <android-base/logging.h>
#include <android-base/parsedouble.h>
#include <android-base/properties.h>
#include <android-base/stringprintf.h>
#include <perfmgr/AdpfConfig.h>
#include <sys/syscall.h>
#include <time.h>
#include <utils/Trace.h>

#include <atomic>

#include "PowerSessionManager.h"

namespace aidl {
namespace google {
namespace hardware {
namespace power {
namespace impl {
namespace pixel {

using ::android::base::StringPrintf;
using ::android::perfmgr::AdpfConfig;
using ::android::perfmgr::HintManager;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

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
        ALOGV("PowerHintSession:%s: skip", __func__);
        return 0;
    }
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

static inline int64_t ns_to_100us(int64_t ns) {
    return ns / 100000;
}

static int64_t convertWorkDurationToBoostByPid(std::shared_ptr<AdpfConfig> adpfConfig,
                                               nanoseconds targetDuration,
                                               const std::vector<WorkDuration> &actualDurations,
                                               int64_t *integral_error, int64_t *previous_error,
                                               const std::string &idstr) {
    uint64_t samplingWindowP = adpfConfig->mSamplingWindowP;
    uint64_t samplingWindowI = adpfConfig->mSamplingWindowI;
    uint64_t samplingWindowD = adpfConfig->mSamplingWindowD;
    int64_t targetDurationNanos = (int64_t)targetDuration.count();
    int64_t length = actualDurations.size();
    int64_t p_start =
            samplingWindowP == 0 || samplingWindowP > length ? 0 : length - samplingWindowP;
    int64_t i_start =
            samplingWindowI == 0 || samplingWindowI > length ? 0 : length - samplingWindowI;
    int64_t d_start =
            samplingWindowD == 0 || samplingWindowD > length ? 0 : length - samplingWindowD;
    int64_t dt = ns_to_100us(targetDurationNanos);
    int64_t err_sum = 0;
    int64_t derivative_sum = 0;
    for (int64_t i = std::min({p_start, i_start, d_start}); i < length; i++) {
        int64_t actualDurationNanos = actualDurations[i].durationNanos;
        if (std::abs(actualDurationNanos) > targetDurationNanos * 20) {
            ALOGW("The actual duration is way far from the target (%" PRId64 " >> %" PRId64 ")",
                  actualDurationNanos, targetDurationNanos);
        }
        // PID control algorithm
        int64_t error = ns_to_100us(actualDurationNanos - targetDurationNanos);
        if (i >= d_start) {
            derivative_sum += error - (*previous_error);
        }
        if (i >= p_start) {
            err_sum += error;
        }
        if (i >= i_start) {
            *integral_error = *integral_error + error * dt;
            *integral_error = std::min(adpfConfig->getPidIHighDivI(), *integral_error);
            *integral_error = std::max(adpfConfig->getPidILowDivI(), *integral_error);
        }
        *previous_error = error;
    }
    int64_t pOut = static_cast<int64_t>((err_sum > 0 ? adpfConfig->mPidPo : adpfConfig->mPidPu) *
                                        err_sum / (length - p_start));
    int64_t iOut = static_cast<int64_t>(adpfConfig->mPidI * (*integral_error));
    int64_t dOut =
            static_cast<int64_t>((derivative_sum > 0 ? adpfConfig->mPidDo : adpfConfig->mPidDu) *
                                 derivative_sum / dt / (length - d_start));

    int64_t output = pOut + iOut + dOut;
    if (ATRACE_ENABLED()) {
        std::string sz = StringPrintf("adpf.%s-pid.err", idstr.c_str());
        ATRACE_INT(sz.c_str(), err_sum / (length - p_start));
        sz = StringPrintf("adpf.%s-pid.integral", idstr.c_str());
        ATRACE_INT(sz.c_str(), *integral_error);
        sz = StringPrintf("adpf.%s-pid.derivative", idstr.c_str());
        ATRACE_INT(sz.c_str(), derivative_sum / dt / (length - d_start));
        sz = StringPrintf("adpf.%s-pid.pOut", idstr.c_str());
        ATRACE_INT(sz.c_str(), pOut);
        sz = StringPrintf("adpf.%s-pid.iOut", idstr.c_str());
        ATRACE_INT(sz.c_str(), iOut);
        sz = StringPrintf("adpf.%s-pid.dOut", idstr.c_str());
        ATRACE_INT(sz.c_str(), dOut);
        sz = StringPrintf("adpf.%s-pid.output", idstr.c_str());
        ATRACE_INT(sz.c_str(), output);
    }
    return output;
}

}  // namespace

PowerHintSession::PowerHintSession(int32_t tgid, int32_t uid, const std::vector<int32_t> &threadIds,
                                   int64_t durationNanos) {
    mDescriptor = new AppHintDesc(tgid, uid, threadIds);
    mDescriptor->duration = std::chrono::nanoseconds(durationNanos);
    mHintTimerHandler = sp<HintTimerHandler>(new HintTimerHandler(this));
    mPowerManagerHandler = PowerSessionManager::getInstance();

    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-target", idstr.c_str());
        ATRACE_INT(sz.c_str(), (int64_t)mDescriptor->duration.count());
        sz = StringPrintf("adpf.%s-active", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->is_active.load());
    }
    PowerSessionManager::getInstance()->addPowerSession(this);
    // init boost
    setUclamp(HintManager::GetInstance()->GetAdpfProfile()->mUclampMinHigh);
    ALOGV("PowerHintSession created: %s", mDescriptor->toString().c_str());
}

PowerHintSession::~PowerHintSession() {
    close();
    ALOGV("PowerHintSession deleted: %s", mDescriptor->toString().c_str());
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-target", idstr.c_str());
        ATRACE_INT(sz.c_str(), 0);
        sz = StringPrintf("adpf.%s-actl_last", idstr.c_str());
        ATRACE_INT(sz.c_str(), 0);
        sz = sz = StringPrintf("adpf.%s-active", idstr.c_str());
        ATRACE_INT(sz.c_str(), 0);
    }
    mHintTimerHandler->setSessionDead();
    delete mDescriptor;
}

std::string PowerHintSession::getIdString() const {
    std::string idstr = StringPrintf("%" PRId32 "-%" PRId32 "-%" PRIxPTR, mDescriptor->tgid,
                                     mDescriptor->uid, reinterpret_cast<uintptr_t>(this) & 0xffff);
    return idstr;
}

void PowerHintSession::updateUniveralBoostMode() {
    if (ATRACE_ENABLED()) {
        const std::string tag = StringPrintf("%s:updateUniveralBoostMode()", getIdString().c_str());
        ATRACE_BEGIN(tag.c_str());
    }
    PowerHintMonitor::getInstance()->getLooper()->sendMessage(mPowerManagerHandler, NULL);
    if (ATRACE_ENABLED()) {
        ATRACE_END();
    }
}

int PowerHintSession::setUclamp(int32_t min, bool update) {
    std::lock_guard<std::mutex> guard(mLock);
    min = std::max(0, min);
    min = std::min(min, kMaxUclampValue);
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-min", idstr.c_str());
        ATRACE_INT(sz.c_str(), min);
    }
    for (const auto tid : mDescriptor->threadIds) {
        sched_attr attr = {};
        attr.size = sizeof(attr);

        attr.sched_flags = (SCHED_FLAG_KEEP_ALL | SCHED_FLAG_UTIL_CLAMP);
        attr.sched_util_min = min;
        attr.sched_util_max = kMaxUclampValue;

        int ret = sched_setattr(tid, &attr, 0);
        if (ret) {
            ALOGW("sched_setattr failed for thread %d, err=%d", tid, errno);
        }
        ALOGV("PowerHintSession tid: %d, uclamp(%d, %d)", tid, min, kMaxUclampValue);
    }
    if (update) {
        mDescriptor->current_min = min;
    }
    mDescriptor->transitioanl_min = min;
    return 0;
}

int PowerHintSession::restoreUclamp() {
    if (mDescriptor->transitioanl_min == mDescriptor->current_min) {
        return 1;
    }
    return setUclamp(mDescriptor->current_min, false);
}

ndk::ScopedAStatus PowerHintSession::pause() {
    if (mSessionClosed) {
        ALOGE("Error: session is dead");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (!mDescriptor->is_active.load())
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    // Reset to default uclamp value.
    setUclamp(0, false);
    mDescriptor->is_active.store(false);
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-active", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->is_active.load());
    }
    updateUniveralBoostMode();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PowerHintSession::resume() {
    if (mSessionClosed) {
        ALOGE("Error: session is dead");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (mDescriptor->is_active.load())
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    mDescriptor->is_active.store(true);
    mHintTimerHandler->updateHintTimer(0);
    // resume boost
    setUclamp(mDescriptor->current_min, false);
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-active", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->is_active.load());
    }
    updateUniveralBoostMode();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PowerHintSession::close() {
    bool sessionClosedExpectedToBe = false;
    if (!mSessionClosed.compare_exchange_strong(sessionClosedExpectedToBe, true)) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    setUclamp(0);
    PowerSessionManager::getInstance()->removePowerSession(this);
    updateUniveralBoostMode();
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PowerHintSession::updateTargetWorkDuration(int64_t targetDurationNanos) {
    if (mSessionClosed) {
        ALOGE("Error: session is dead");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (targetDurationNanos <= 0) {
        ALOGE("Error: targetDurationNanos(%" PRId64 ") should bigger than 0", targetDurationNanos);
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    targetDurationNanos =
            targetDurationNanos * HintManager::GetInstance()->GetAdpfProfile()->mTargetTimeFactor;
    ALOGV("update target duration: %" PRId64 " ns", targetDurationNanos);

    mDescriptor->duration = std::chrono::nanoseconds(targetDurationNanos);
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-target", idstr.c_str());
        ATRACE_INT(sz.c_str(), (int64_t)mDescriptor->duration.count());
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus PowerHintSession::reportActualWorkDuration(
        const std::vector<WorkDuration> &actualDurations) {
    if (mSessionClosed) {
        ALOGE("Error: session is dead");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (mDescriptor->duration.count() == 0LL) {
        ALOGE("Expect to call updateTargetWorkDuration() first.");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (actualDurations.size() == 0) {
        ALOGE("Error: duration.size() shouldn't be %zu.", actualDurations.size());
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    }
    if (!mDescriptor->is_active.load()) {
        ALOGE("Error: shouldn't report duration during pause state.");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    mDescriptor->update_count++;
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-batch_size", idstr.c_str());
        ATRACE_INT(sz.c_str(), actualDurations.size());
        sz = StringPrintf("adpf.%s-actl_last", idstr.c_str());
        ATRACE_INT(sz.c_str(), actualDurations.back().durationNanos);
        sz = StringPrintf("adpf.%s-target", idstr.c_str());
        ATRACE_INT(sz.c_str(), (int64_t)mDescriptor->duration.count());
        sz = StringPrintf("adpf.%s-hint.count", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->update_count);
        sz = StringPrintf("adpf.%s-hint.overtime", idstr.c_str());
        ATRACE_INT(sz.c_str(),
                   actualDurations.back().durationNanos - mDescriptor->duration.count() > 0);
    }

    if (PowerHintMonitor::getInstance()->isRunning() && isStale()) {
        mDescriptor->integral_error =
                std::max(adpfConfig->getPidIInitDivI(), mDescriptor->integral_error);
        if (ATRACE_ENABLED()) {
            const std::string idstr = getIdString();
            std::string sz = StringPrintf("adpf.%s-wakeup", idstr.c_str());
            ATRACE_INT(sz.c_str(), mDescriptor->integral_error);
            ATRACE_INT(sz.c_str(), 0);
        }
    }

    mHintTimerHandler->updateHintTimer(actualDurations);

    if (!adpfConfig->mPidOn) {
        setUclamp(adpfConfig->mUclampMinHigh);
        return ndk::ScopedAStatus::ok();
    }
    int64_t output = convertWorkDurationToBoostByPid(
            adpfConfig, mDescriptor->duration, actualDurations, &(mDescriptor->integral_error),
            &(mDescriptor->previous_error), getIdString());

    /* apply to all the threads in the group */
    if (output != 0) {
        int next_min = std::min(static_cast<int>(adpfConfig->mUclampMinHigh),
                                mDescriptor->current_min + static_cast<int>(output));
        next_min = std::max(static_cast<int>(adpfConfig->mUclampMinLow), next_min);
        if (std::abs(mDescriptor->current_min - next_min) > adpfConfig->mUclampMinGranularity) {
            setUclamp(next_min);
        } else {
            restoreUclamp();
        }
    }

    return ndk::ScopedAStatus::ok();
}

std::string AppHintDesc::toString() const {
    std::string out =
            StringPrintf("session %" PRIxPTR "\n", reinterpret_cast<uintptr_t>(this) & 0xffff);
    const int64_t durationNanos = duration.count();
    out.append(StringPrintf("  duration: %" PRId64 " ns\n", durationNanos));
    out.append(StringPrintf("  uclamp.min: %d \n", current_min));
    out.append(StringPrintf("  uid: %d, tgid: %d\n", uid, tgid));

    out.append("  threadIds: [");
    bool first = true;
    for (int tid : threadIds) {
        if (!first) {
            out.append(", ");
        }
        out.append(std::to_string(tid));
        first = false;
    }
    out.append("]\n");
    return out;
}

bool PowerHintSession::isActive() {
    return mDescriptor->is_active.load();
}

bool PowerHintSession::isStale() {
    auto now = std::chrono::steady_clock::now();
    return now >= mHintTimerHandler->getStaleTime();
}

const std::vector<int> &PowerHintSession::getTidList() const {
    return mDescriptor->threadIds;
}

void PowerHintSession::setStale() {
    // Reset to default uclamp value.
    setUclamp(0, false);
    // Deliver a task to check if all sessions are inactive.
    updateUniveralBoostMode();
}

void PowerHintSession::HintTimerHandler::updateHintTimer(int64_t actualDurationNs) {
    std::lock_guard<std::mutex> guard(mStaleLock);
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    HintTimerState prevState = mState;
    mState = MONITORING;
    auto now = std::chrono::steady_clock::now();
    mLastUpdatedTime.store(now);
    nanoseconds nextStartDur = nanoseconds((mSession->mDescriptor->work_period
                                                    ? mSession->mDescriptor->work_period
                                                    : mSession->mDescriptor->duration.count()) -
                                           actualDurationNs);
    mNextStartTime.store(now + nextStartDur);
    if (prevState != MONITORING) {
        int64_t next =
                static_cast<int64_t>(duration_cast<nanoseconds>(getEarlyBoostTime() - now).count());
        PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mHintTimerHandler);
        PowerHintMonitor::getInstance()->getLooper()->sendMessageDelayed(
                next, mSession->mHintTimerHandler, NULL);
        if (prevState == STALE) {
            mSession->updateUniveralBoostMode();
        }
    }
    if (ATRACE_ENABLED()) {
        const std::string idstr = mSession->getIdString();
        std::string sz = StringPrintf("adpf.%s-timer.state", idstr.c_str());
        ATRACE_INT(sz.c_str(), mState);
        sz = StringPrintf("adpf.%s-timer.nextvsync", idstr.c_str());
        ATRACE_INT(sz.c_str(), nextStartDur.count());
        sz = StringPrintf("adpf.%s-timer.nexthint", idstr.c_str());
        ATRACE_INT(sz.c_str(),
                   (int64_t)(nextStartDur.count() + mSession->mDescriptor->duration.count() *
                                                            adpfConfig->mEarlyBoostTimeFactor));
    }
}

void PowerHintSession::HintTimerHandler::updateHintTimer(
        const std::vector<WorkDuration> &actualDurations) {
    if (actualDurations.size() == 0)
        return;
    if (actualDurations.size() >= 2) {
        const WorkDuration &last = actualDurations[actualDurations.size() - 2];
        mSession->mDescriptor->last_start = last.timeStampNanos - last.durationNanos;
        ALOGD("last_start initialized by previous.");
    }
    const WorkDuration &current = actualDurations.back();
    int64_t curr_start = current.timeStampNanos - current.durationNanos;
    if (!mSession->mDescriptor->last_start) {
        mSession->mDescriptor->last_start = curr_start;
        ALOGD("last_start initialized by current.");
        updateHintTimer(current.durationNanos);
        return;
    }
    int64_t period = curr_start - mSession->mDescriptor->last_start;
    mSession->mDescriptor->last_start = curr_start;
    if (period > 0 && period < mSession->mDescriptor->duration.count() * 2) {
        // Accounting workload period with moving average for the last 10 workload.
        mSession->mDescriptor->work_period =
                0.9 * mSession->mDescriptor->work_period + 0.1 * period;
        if (ATRACE_ENABLED()) {
            const std::string idstr = mSession->getIdString();
            std::string sz = StringPrintf("adpf.%s-timer.period", idstr.c_str());
            ATRACE_INT(sz.c_str(), period);
        }
    }
    updateHintTimer(current.durationNanos);
}

time_point<steady_clock> PowerHintSession::HintTimerHandler::getEarlyBoostTime() {
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    if (!adpfConfig->mEarlyBoostOn) {
        return getStaleTime();
    }
    int64_t earlyBoostTimeoutNs =
            (int64_t)mSession->mDescriptor->duration.count() * adpfConfig->mEarlyBoostTimeFactor;
    return mNextStartTime.load() + nanoseconds(earlyBoostTimeoutNs);
}

time_point<steady_clock> PowerHintSession::HintTimerHandler::getStaleTime() {
    return mLastUpdatedTime.load() +
           nanoseconds(static_cast<int64_t>(
                   mSession->mDescriptor->duration.count() *
                   HintManager::GetInstance()->GetAdpfProfile()->mStaleTimeFactor));
}

PowerHintSession::HintTimerHandler::~HintTimerHandler() {
    ATRACE_CALL();
}

void PowerHintSession::HintTimerHandler::handleMessage(const Message &) {
    std::lock_guard<std::mutex> guard(mStaleLock);
    if (mIsSessionDead) {
        return;
    }
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    auto now = std::chrono::steady_clock::now();
    auto staleTime = getStaleTime();
    auto earlyBoostTime = getEarlyBoostTime();
    if (adpfConfig->mEarlyBoostOn && now < earlyBoostTime) {
        int64_t next =
                static_cast<int64_t>(duration_cast<nanoseconds>(earlyBoostTime - now).count());
        mState = MONITORING;
        // Schedule for the early hint check.
        PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mHintTimerHandler);
        PowerHintMonitor::getInstance()->getLooper()->sendMessageDelayed(
                next, mSession->mHintTimerHandler, NULL);
        if (ATRACE_ENABLED()) {
            const std::string idstr = mSession->getIdString();
            std::string sz = StringPrintf("adpf.%s-timer.nexthint", idstr.c_str());
            ATRACE_INT(sz.c_str(), next);
        }
    } else if (now >= staleTime) {  // Check if the session is stale.
        mSession->setStale();
        mState = STALE;
    } else {  // Check if it's time to do early boost.
        if (adpfConfig->mEarlyBoostOn) {
            mState = EARLY_BOOST;
            mSession->setUclamp(adpfConfig->mUclampMinHigh);
        }
        int64_t next = static_cast<int64_t>(duration_cast<nanoseconds>(staleTime - now).count());
        // Schedule for the stale timeout check.
        PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mHintTimerHandler);
        PowerHintMonitor::getInstance()->getLooper()->sendMessageDelayed(
                next, mSession->mHintTimerHandler, NULL);
    }
    if (ATRACE_ENABLED()) {
        const std::string idstr = mSession->getIdString();
        std::string sz = StringPrintf("adpf.%s-timer.state", idstr.c_str());
        ATRACE_INT(sz.c_str(), mState);
    }
}

void PowerHintSession::HintTimerHandler::setSessionDead() {
    std::lock_guard<std::mutex> guard(mStaleLock);
    PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mHintTimerHandler);
    mIsSessionDead = true;
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
