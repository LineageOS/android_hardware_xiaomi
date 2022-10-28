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
#include <private/android_filesystem_config.h>
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
    mStaleTimerHandler = sp<StaleTimerHandler>(new StaleTimerHandler(this));
    mPowerManagerHandler = PowerSessionManager::getInstance();
    mLastUpdatedTime.store(std::chrono::steady_clock::now());

    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-target", idstr.c_str());
        ATRACE_INT(sz.c_str(), (int64_t)mDescriptor->duration.count());
        sz = StringPrintf("adpf.%s-active", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->is_active.load());
    }
    PowerSessionManager::getInstance()->addPowerSession(this);
    // init boost
    setSessionUclampMin(HintManager::GetInstance()->GetAdpfProfile()->mUclampMinInit);
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
    delete mDescriptor;
}

std::string PowerHintSession::getIdString() const {
    std::string idstr = StringPrintf("%" PRId32 "-%" PRId32 "-%" PRIxPTR, mDescriptor->tgid,
                                     mDescriptor->uid, reinterpret_cast<uintptr_t>(this) & 0xffff);
    return idstr;
}

bool PowerHintSession::isAppSession() {
    // Check if uid is in range reserved for applications
    return mDescriptor->uid >= AID_APP_START;
}

void PowerHintSession::updateUniveralBoostMode() {
    if (!isAppSession()) {
        return;
    }
    if (ATRACE_ENABLED()) {
        const std::string tag = StringPrintf("%s:updateUniveralBoostMode()", getIdString().c_str());
        ATRACE_BEGIN(tag.c_str());
    }
    PowerHintMonitor::getInstance()->getLooper()->sendMessage(mPowerManagerHandler, NULL);
    if (ATRACE_ENABLED()) {
        ATRACE_END();
    }
}

void PowerHintSession::tryToSendPowerHint(std::string hint) {
    if (!mSupportedHints[hint].has_value()) {
        mSupportedHints[hint] = HintManager::GetInstance()->IsHintSupported(hint);
    }
    if (mSupportedHints[hint].value()) {
        HintManager::GetInstance()->DoHint(hint);
    }
}

int PowerHintSession::setSessionUclampMin(int32_t min) {
    {
        std::lock_guard<std::mutex> guard(mSessionLock);
        mDescriptor->current_min = min;
    }
    if (min) {
        mStaleTimerHandler->updateTimer();
    }
    PowerSessionManager::getInstance()->setUclampMin(this, min);

    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-min", idstr.c_str());
        ATRACE_INT(sz.c_str(), min);
    }
    return 0;
}

int PowerHintSession::getUclampMin() {
    return mDescriptor->current_min;
}

void PowerHintSession::dumpToStream(std::ostream &stream) {
    stream << "ID.Min.Act.Timeout(" << getIdString();
    stream << ", " << mDescriptor->current_min;
    stream << ", " << mDescriptor->is_active;
    stream << ", " << isTimeout() << ")";
}

ndk::ScopedAStatus PowerHintSession::pause() {
    if (mSessionClosed) {
        ALOGE("Error: session is dead");
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }
    if (!mDescriptor->is_active.load())
        return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    // Reset to default uclamp value.
    mDescriptor->is_active.store(false);
    setStale();
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
    // resume boost
    setSessionUclampMin(mDescriptor->current_min);
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
    // Remove the session from PowerSessionManager first to avoid racing.
    PowerSessionManager::getInstance()->removePowerSession(this);
    mStaleTimerHandler->setSessionDead();
    setSessionUclampMin(0);
    mDescriptor->is_active.store(false);
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
    bool isFirstFrame = isTimeout();
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

    mLastUpdatedTime.store(std::chrono::steady_clock::now());
    if (isFirstFrame) {
        if (isAppSession()) {
            tryToSendPowerHint("ADPF_FIRST_FRAME");
        }
        updateUniveralBoostMode();
    }

    if (!adpfConfig->mPidOn) {
        setSessionUclampMin(adpfConfig->mUclampMinHigh);
        return ndk::ScopedAStatus::ok();
    }
    int64_t output = convertWorkDurationToBoostByPid(
            adpfConfig, mDescriptor->duration, actualDurations, &(mDescriptor->integral_error),
            &(mDescriptor->previous_error), getIdString());

    /* apply to all the threads in the group */
    int next_min = std::min(static_cast<int>(adpfConfig->mUclampMinHigh),
                            mDescriptor->current_min + static_cast<int>(output));
    next_min = std::max(static_cast<int>(adpfConfig->mUclampMinLow), next_min);
    setSessionUclampMin(next_min);
    mStaleTimerHandler->updateTimer(getStaleTime());

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

bool PowerHintSession::isTimeout() {
    auto now = std::chrono::steady_clock::now();
    return now >= getStaleTime();
}

const std::vector<int> &PowerHintSession::getTidList() const {
    return mDescriptor->threadIds;
}

void PowerHintSession::setStale() {
    // Reset to default uclamp value.
    PowerSessionManager::getInstance()->setUclampMin(this, 0);
    // Deliver a task to check if all sessions are inactive.
    updateUniveralBoostMode();
    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-min", idstr.c_str());
        ATRACE_INT(sz.c_str(), 0);
    }
}

void PowerHintSession::wakeup() {
    std::lock_guard<std::mutex> guard(mSessionLock);

    // We only wake up non-paused session
    if (mSessionClosed || !isActive()) {
        return;
    }
    // Update session's timer
    mStaleTimerHandler->updateTimer();
    // Skip uclamp update for stale session
    if (!isTimeout()) {
        return;
    }
    if (ATRACE_ENABLED()) {
        std::string tag = StringPrintf("wakeup.%s(a:%d,s:%d)", getIdString().c_str(), isActive(),
                                       isTimeout());
        ATRACE_NAME(tag.c_str());
    }
    std::shared_ptr<AdpfConfig> adpfConfig = HintManager::GetInstance()->GetAdpfProfile();
    mDescriptor->current_min =
            std::max(mDescriptor->current_min, static_cast<int>(adpfConfig->mUclampMinInit));

    if (ATRACE_ENABLED()) {
        const std::string idstr = getIdString();
        std::string sz = StringPrintf("adpf.%s-min", idstr.c_str());
        ATRACE_INT(sz.c_str(), mDescriptor->current_min);
    }
}

time_point<steady_clock> PowerHintSession::getStaleTime() {
    return mLastUpdatedTime.load() +
           nanoseconds(static_cast<int64_t>(
                   mDescriptor->duration.count() *
                   HintManager::GetInstance()->GetAdpfProfile()->mStaleTimeFactor));
}

void PowerHintSession::StaleTimerHandler::updateTimer() {
    time_point<steady_clock> staleTime =
            std::chrono::steady_clock::now() +
            nanoseconds(static_cast<int64_t>(
                    mSession->mDescriptor->duration.count() *
                    HintManager::GetInstance()->GetAdpfProfile()->mStaleTimeFactor));
    updateTimer(staleTime);
}

void PowerHintSession::StaleTimerHandler::updateTimer(time_point<steady_clock> staleTime) {
    mStaleTime.store(staleTime);
    {
        std::lock_guard<std::mutex> guard(mMessageLock);
        PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mStaleTimerHandler);
        PowerHintMonitor::getInstance()->getLooper()->sendMessage(mSession->mStaleTimerHandler,
                                                                  NULL);
    }
    mIsMonitoring.store(true);
    if (ATRACE_ENABLED()) {
        const std::string idstr = mSession->getIdString();
        std::string sz = StringPrintf("adpf.%s-timer.stale", idstr.c_str());
        ATRACE_INT(sz.c_str(), 0);
    }
}

void PowerHintSession::StaleTimerHandler::handleMessage(const Message &) {
    std::lock_guard<std::mutex> guard(mClosedLock);
    if (mIsSessionDead) {
        return;
    }
    auto now = std::chrono::steady_clock::now();
    int64_t next =
            static_cast<int64_t>(duration_cast<nanoseconds>(mStaleTime.load() - now).count());
    if (next > 0) {
        // Schedule for the stale timeout check.
        std::lock_guard<std::mutex> guard(mMessageLock);
        PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mStaleTimerHandler);
        PowerHintMonitor::getInstance()->getLooper()->sendMessageDelayed(
                next, mSession->mStaleTimerHandler, NULL);
    } else {
        mSession->setStale();
        mIsMonitoring.store(false);
    }
    if (ATRACE_ENABLED()) {
        const std::string idstr = mSession->getIdString();
        std::string sz = StringPrintf("adpf.%s-timer.stale", idstr.c_str());
        ATRACE_INT(sz.c_str(), mIsMonitoring ? 0 : 1);
    }
}

void PowerHintSession::StaleTimerHandler::setSessionDead() {
    std::lock_guard<std::mutex> guard(mClosedLock);
    mIsSessionDead = true;
    PowerHintMonitor::getInstance()->getLooper()->removeMessages(mSession->mStaleTimerHandler);
}

}  // namespace pixel
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace google
}  // namespace aidl
