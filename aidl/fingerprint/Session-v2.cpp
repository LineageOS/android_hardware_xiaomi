/*
 * Copyright (C) 2024-2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <thread>

#include "Legacy2Aidl.h"
#include "Session.h"

#include "CancellationSignal.h"

#ifdef IMPL_V2

namespace aidl::android::hardware::biometrics::fingerprint {

void onClientDeathV2(void* cookie) {
    ALOGI("FingerprintService has died");
    SessionV2* session = static_cast<SessionV2*>(cookie);
    if (session && !session->isClosed()) {
        session->close();
    }
}

SessionV2::SessionV2(fingerprint_device_t* device, UdfpsHandler* udfpsHandler, int userId,
                     std::shared_ptr<ISessionCallback> cb, LockoutTracker lockoutTracker)
    : mDevice(device),
      mLockoutTracker(lockoutTracker),
      mUserId(userId),
      mCb(cb),
      mUdfpsHandler(udfpsHandler) {
    mDeathRecipient = AIBinder_DeathRecipient_new(onClientDeathV2);

    auto path = std::format("/data/vendor_de/{}/fpdata/", userId);
    mDevice->set_active_group(mDevice, mUserId, path.c_str());
}

ndk::ScopedAStatus SessionV2::generateChallenge() {
    uint64_t challenge = mDevice->generate_challenge(mDevice);
    ALOGI("generateChallenge: %ld", challenge);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::revokeChallenge(int64_t challenge) {
    ALOGI("revokeChallenge: %ld", challenge);

    mDevice->revoke_challenge(mDevice, challenge);

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::enroll(const HardwareAuthToken& hat,
                                     std::shared_ptr<ICancellationSignal>* out) {
    hw_auth_token_t authToken;
    translate(hat, authToken);
    int error = mDevice->enroll(mDevice, &authToken);
    if (error) {
        ALOGE("enroll failed: %d", error);
        mCb->onError(Error::UNABLE_TO_PROCESS, error);
    }

    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::authenticate(int64_t operationId,
                                           std::shared_ptr<ICancellationSignal>* out) {
    checkSensorLockout();
    int error = mDevice->authenticate(mDevice, operationId);
    if (error) {
        ALOGE("authenticate failed: %d", error);
        mCb->onError(Error::UNABLE_TO_PROCESS, error);
    }

    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::detectInteraction(std::shared_ptr<ICancellationSignal>* out) {
    ALOGD("Detect interaction is not supported");
    mCb->onError(Error::UNABLE_TO_PROCESS, 0 /* vendorCode */);

    *out = SharedRefBase::make<CancellationSignal>(this);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::enumerateEnrollments() {
    int error = mDevice->enumerate(mDevice);
    if (error) {
        ALOGE("enumerate failed: %d", error);
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::removeEnrollments(const std::vector<int32_t>& enrollmentIds) {
    ALOGI("removeEnrollments, size: %zu", enrollmentIds.size());

    std::vector<uint32_t> fids(enrollmentIds.begin(), enrollmentIds.end());
    int error = mDevice->remove(mDevice, fids.data(), static_cast<uint32_t>(fids.size()));
    if (error) {
        ALOGE("Failed to remove enrollments: %d", error);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::getAuthenticatorId() {
    uint64_t auth_id = mDevice->get_authenticator_id(mDevice);
    ALOGI("getAuthenticatorId: %ld", auth_id);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::invalidateAuthenticatorId() {
    uint64_t auth_id = mDevice->invalidate_authenticator_id(mDevice);
    ALOGI("invalidateAuthenticatorId: %ld", auth_id);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::resetLockout(const HardwareAuthToken& hat) {
    hw_auth_token_t authToken;
    translate(hat, authToken);

    int resetResult = mDevice->reset_lockout(mDevice, &authToken);
    if (resetResult != 0) {
        ALOGE("Failed to reset lockout: %d", resetResult);
    }

    clearLockout(true);
    if (mIsLockoutTimerStarted) mIsLockoutTimerAborted = true;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::onPointerDown(int32_t /*pointerId*/, int32_t x, int32_t y,
                                            float minor, float major) {
    if (mUdfpsHandler) {
        mUdfpsHandler->onFingerDown(x, y, minor, major);
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::onPointerUp(int32_t /*pointerId*/) {
    if (mUdfpsHandler) {
        mUdfpsHandler->onFingerUp();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::onUiReady() {
    // TODO: stub

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::authenticateWithContext(
        int64_t operationId, const common::OperationContext& /*context*/,
        std::shared_ptr<common::ICancellationSignal>* out) {
    return authenticate(operationId, out);
}

ndk::ScopedAStatus SessionV2::enrollWithContext(const keymaster::HardwareAuthToken& hat,
                                                const common::OperationContext& /*context*/,
                                                std::shared_ptr<common::ICancellationSignal>* out) {
    return enroll(hat, out);
}

ndk::ScopedAStatus SessionV2::detectInteractionWithContext(
        const common::OperationContext& /*context*/,
        std::shared_ptr<common::ICancellationSignal>* out) {
    return detectInteraction(out);
}

ndk::ScopedAStatus SessionV2::onPointerDownWithContext(const PointerContext& context) {
    return onPointerDown(context.pointerId, context.x, context.y, context.minor, context.major);
}

ndk::ScopedAStatus SessionV2::onPointerUpWithContext(const PointerContext& context) {
    return onPointerUp(context.pointerId);
}

ndk::ScopedAStatus SessionV2::onContextChanged(const common::OperationContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::onPointerCancelWithContext(const PointerContext& /*context*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::setIgnoreDisplayTouches(bool /*shouldIgnore*/) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SessionV2::cancel() {
    if (mUdfpsHandler) {
        mUdfpsHandler->cancel();
    }

    int ret = mDevice->cancel(mDevice);

    if (ret == 0) {
        mCb->onError(Error::CANCELED, 0 /* vendorCode */);
        return ndk::ScopedAStatus::ok();
    }

    return ndk::ScopedAStatus::fromServiceSpecificError(ret);
}

ndk::ScopedAStatus SessionV2::close() {
    mClosed = true;
    mCb->onSessionClosed();
    AIBinder_DeathRecipient_delete(mDeathRecipient);
    return ndk::ScopedAStatus::ok();
}

binder_status_t SessionV2::linkToDeath(AIBinder* binder) {
    return AIBinder_linkToDeath(binder, mDeathRecipient, this);
}

bool SessionV2::isClosed() {
    return mClosed;
}

// Translate from errors returned by traditional HAL (see fingerprint.h) to
// AIDL-compliant Error
Error SessionV2::VendorErrorFilter(int32_t error, int32_t* vendorCode) {
    *vendorCode = 0;

    switch (error) {
        case FINGERPRINT_ERROR_HW_UNAVAILABLE:
            return Error::HW_UNAVAILABLE;
        case FINGERPRINT_ERROR_UNABLE_TO_PROCESS:
            return Error::UNABLE_TO_PROCESS;
        case FINGERPRINT_ERROR_TIMEOUT:
            return Error::TIMEOUT;
        case FINGERPRINT_ERROR_NO_SPACE:
            return Error::NO_SPACE;
        case FINGERPRINT_ERROR_CANCELED:
            return Error::CANCELED;
        case FINGERPRINT_ERROR_UNABLE_TO_REMOVE:
            return Error::UNABLE_TO_REMOVE;
        case FINGERPRINT_ERROR_LOCKOUT: {
            *vendorCode = FINGERPRINT_ERROR_LOCKOUT;
            return Error::VENDOR;
        }
        default:
            if (error >= FINGERPRINT_ERROR_VENDOR_BASE) {
                // vendor specific code.
                *vendorCode = error - FINGERPRINT_ERROR_VENDOR_BASE;
                return Error::VENDOR;
            }
    }
    ALOGE("Unknown error from fingerprint vendor library: %d", error);
    return Error::UNABLE_TO_PROCESS;
}

// Translate acquired messages returned by traditional HAL (see fingerprint.h)
// to AIDL-compliant AcquiredInfo
AcquiredInfo SessionV2::VendorAcquiredFilter(int32_t info, int32_t* vendorCode) {
    *vendorCode = 0;

    switch (info) {
        case FINGERPRINT_ACQUIRED_GOOD:
            return AcquiredInfo::GOOD;
        case FINGERPRINT_ACQUIRED_PARTIAL:
            return AcquiredInfo::PARTIAL;
        case FINGERPRINT_ACQUIRED_INSUFFICIENT:
            return AcquiredInfo::INSUFFICIENT;
        case FINGERPRINT_ACQUIRED_IMAGER_DIRTY:
            return AcquiredInfo::SENSOR_DIRTY;
        case FINGERPRINT_ACQUIRED_TOO_SLOW:
            return AcquiredInfo::TOO_SLOW;
        case FINGERPRINT_ACQUIRED_TOO_FAST:
            return AcquiredInfo::TOO_FAST;
        default:
            if (info >= FINGERPRINT_ACQUIRED_VENDOR_BASE) {
                // vendor specific code.
                *vendorCode = info - FINGERPRINT_ACQUIRED_VENDOR_BASE;
                return AcquiredInfo::VENDOR;
            }
    }
    ALOGE("Unknown acquired message from fingerprint vendor library: %d", info);
    return AcquiredInfo::UNKNOWN;
}

bool SessionV2::checkSensorLockout() {
    LockoutTracker::LockoutMode lockoutMode = mLockoutTracker.getMode();
    if (lockoutMode == LockoutTracker::LockoutMode::kPermanent) {
        ALOGE("Fail: lockout permanent");
        mCb->onLockoutPermanent();
        mIsLockoutTimerAborted = true;
        return true;
    }
    if (lockoutMode == LockoutTracker::LockoutMode::kTimed) {
        int64_t timeLeft = mLockoutTracker.getLockoutTimeLeft();
        ALOGE("Fail: lockout timed: %ld", timeLeft);
        mCb->onLockoutTimed(timeLeft);
        if (!mIsLockoutTimerStarted) startLockoutTimer(timeLeft);
        return true;
    }
    return false;
}

void SessionV2::clearLockout(bool clearAttemptCounter) {
    mLockoutTracker.reset(clearAttemptCounter);
    mCb->onLockoutCleared();
}

void SessionV2::startLockoutTimer(int64_t timeout) {
    std::function<void()> action = std::bind(&SessionV2::lockoutTimerExpired, this);
    std::thread([timeout, action]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        action();
    }).detach();

    mIsLockoutTimerStarted = true;
}

void SessionV2::lockoutTimerExpired() {
    if (!mIsLockoutTimerAborted) clearLockout(false);

    mIsLockoutTimerStarted = false;
    mIsLockoutTimerAborted = false;
}

void SessionV2::notify(const fingerprint_msg_t* msg) {
    // const uint64_t devId = reinterpret_cast<uint64_t>(mDevice);
    switch (msg->type) {
        case FINGERPRINT_ERROR: {
            int32_t vendorCode = 0;
            Error result = VendorErrorFilter(msg->data.error, &vendorCode);
            ALOGD("onError(%hhd, %d)", result, vendorCode);
            mCb->onError(result, vendorCode);
        } break;
        case FINGERPRINT_ACQUIRED: {
            int32_t vendorCode = 0;
            AcquiredInfo result =
                    VendorAcquiredFilter(msg->data.acquired.acquired_info, &vendorCode);
            ALOGD("onAcquired(%hhd, %d)", result, vendorCode);
            if (mUdfpsHandler) {
                mUdfpsHandler->onAcquired(static_cast<int32_t>(result), vendorCode);
            }
            // don't process vendor messages further since frameworks try to disable
            // udfps display mode on vendor acquired messages but our sensors send a
            // vendor message during processing...
            if (result != AcquiredInfo::VENDOR) {
                mCb->onAcquired(result, vendorCode);
            }
        } break;
        case FINGERPRINT_TEMPLATE_ENROLLING: {
            ALOGD("onEnrollResult(fid=%d, rem=%d)", msg->data.enroll.finger.fid,
                  msg->data.enroll.samples_remaining);
            mCb->onEnrollmentProgress(msg->data.enroll.finger.fid,
                                      msg->data.enroll.samples_remaining);
        } break;
        case FINGERPRINT_TEMPLATE_REMOVED: {
            ALOGD("onRemove(fid=%d, rem=%d)", msg->data.removed.finger.fid,
                  msg->data.removed.remaining_templates);
            std::vector<int> enrollments;
            enrollments.push_back(msg->data.removed.finger.fid);
            mCb->onEnrollmentsRemoved(enrollments);
        } break;
        case FINGERPRINT_AUTHENTICATED: {
            ALOGD("onAuthenticated(fid=%d)", msg->data.authenticated.finger.fid);
            if (msg->data.authenticated.finger.fid != 0) {
                const hw_auth_token_t hat = msg->data.authenticated.hat;
                HardwareAuthToken authToken;
                translate(hat, authToken);

                if (mUdfpsHandler) {
                    mUdfpsHandler->onAuthenticationSucceeded();
                }
                mCb->onAuthenticationSucceeded(msg->data.authenticated.finger.fid, authToken);
                mLockoutTracker.reset(true);
            } else {
                if (mUdfpsHandler) {
                    mUdfpsHandler->onAuthenticationFailed();
                }
                mCb->onAuthenticationFailed();
                mLockoutTracker.addFailedAttempt();
                checkSensorLockout();
            }
        } break;
        case FINGERPRINT_TEMPLATE_ENUMERATING: {
            ALOGD("onEnumerate(fid=%d, rem=%d)", msg->data.enumerated.finger.fid,
                  msg->data.enumerated.remaining_templates);
            static std::vector<int> enrollments;
            enrollments.push_back(msg->data.enumerated.finger.fid);
            if (msg->data.enumerated.remaining_templates == 0) {
                mCb->onEnrollmentsEnumerated(enrollments);
                enrollments.clear();
            }
        } break;
        case FINGERPRINT_CHALLENGE_GENERATED: {
            ALOGD("onChallengeGenerated(%lu)", msg->data.challenge.value);
            mCb->onChallengeGenerated(msg->data.challenge.value);
        } break;
        case FINGERPRINT_CHALLENGE_REVOKED: {
            ALOGD("onChallengeRevoked(%lu)", msg->data.challenge.value);
            mCb->onChallengeRevoked(msg->data.challenge.value);
        } break;
        case FINGERPRINT_AUTHENTICATOR_ID_RETRIEVED: {
            ALOGD("onAuthenticatorIdRetrieved(%lu)", msg->data.authenticator.id);
            mCb->onAuthenticatorIdRetrieved(msg->data.authenticator.id);
        } break;
        case FINGERPRINT_AUTHENTICATOR_ID_INVALIDATED: {
            ALOGD("onAuthenticatorIdInvalidated(%lu)", msg->data.authenticator.id);
            mCb->onAuthenticatorIdInvalidated(msg->data.authenticator.id);
        } break;
        case FINGERPRINT_RESET_LOCKOUT: {
            ALOGD("onLockoutCleared");
            clearLockout(true);
        } break;
    }
}

}  // namespace aidl::android::hardware::biometrics::fingerprint

#endif
