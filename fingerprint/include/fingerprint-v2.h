/*
 * Copyright (C) 2014 The Android Open Source Project
 *               2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/hardware.h>
#include <hardware/hw_auth_token.h>

#define FINGERPRINT_MODULE_API_VERSION_1_0 HARDWARE_MODULE_API_VERSION(1, 0)
#define FINGERPRINT_MODULE_API_VERSION_2_0 HARDWARE_MODULE_API_VERSION(2, 0)
#define FINGERPRINT_MODULE_API_VERSION_2_1 HARDWARE_MODULE_API_VERSION(2, 1)
#define FINGERPRINT_MODULE_API_VERSION_3_0 HARDWARE_MODULE_API_VERSION(3, 0)
#define FINGERPRINT_HARDWARE_MODULE_ID "fingerprint"

typedef enum fingerprint_msg_type {
    FINGERPRINT_ERROR = -1,
    FINGERPRINT_ACQUIRED = 1,
    FINGERPRINT_TEMPLATE_ENROLLING = 3,
    FINGERPRINT_TEMPLATE_REMOVED = 4,
    FINGERPRINT_AUTHENTICATED = 5,
    FINGERPRINT_TEMPLATE_ENUMERATING = 6,
    FINGERPRINT_CHALLENGE_GENERATED = 7,
    FINGERPRINT_CHALLENGE_REVOKED = 8,
    FINGERPRINT_AUTHENTICATOR_ID_RETRIEVED = 9,
    FINGERPRINT_AUTHENTICATOR_ID_INVALIDATED = 10,
    FINGERPRINT_RESET_LOCKOUT = 11,
} fingerprint_msg_type_t;

/*
 * Fingerprint errors are meant to tell the framework to terminate the current operation and ask
 * for the user to correct the situation. These will almost always result in messaging and user
 * interaction to correct the problem.
 *
 * For example, FINGERPRINT_ERROR_CANCELED should follow any acquisition message that results in
 * a situation where the current operation can't continue without user interaction. For example,
 * if the sensor is dirty during enrollment and no further enrollment progress can be made,
 * send FINGERPRINT_ACQUIRED_IMAGER_DIRTY followed by FINGERPRINT_ERROR_CANCELED.
 */
typedef enum fingerprint_error {
    FINGERPRINT_ERROR_HW_UNAVAILABLE = 1,    /* The hardware has an error that can't be resolved. */
    FINGERPRINT_ERROR_UNABLE_TO_PROCESS = 2, /* Bad data; operation can't continue */
    FINGERPRINT_ERROR_TIMEOUT = 3,  /* The operation has timed out waiting for user input. */
    FINGERPRINT_ERROR_NO_SPACE = 4, /* No space available to store a template */
    FINGERPRINT_ERROR_CANCELED = 5, /* The current operation can't proceed. See above. */
    FINGERPRINT_ERROR_UNABLE_TO_REMOVE = 6, /* fingerprint with given id can't be removed */
    FINGERPRINT_ERROR_LOCKOUT =
            7, /* the fingerprint hardware is in lockout due to too many attempts */
    FINGERPRINT_ERROR_VENDOR_BASE = 1000 /* vendor-specific error messages start here */
} fingerprint_error_t;

/*
 * Fingerprint acquisition info is meant as feedback for the current operation.  Anything but
 * FINGERPRINT_ACQUIRED_GOOD will be shown to the user as feedback on how to take action on the
 * current operation. For example, FINGERPRINT_ACQUIRED_IMAGER_DIRTY can be used to tell the user
 * to clean the sensor.  If this will cause the current operation to fail, an additional
 * FINGERPRINT_ERROR_CANCELED can be sent to stop the operation in progress (e.g. enrollment).
 * In general, these messages will result in a "Try again" message.
 */
typedef enum fingerprint_acquired_info {
    FINGERPRINT_ACQUIRED_GOOD = 0,
    FINGERPRINT_ACQUIRED_PARTIAL = 1,      /* sensor needs more data, i.e. longer swipe. */
    FINGERPRINT_ACQUIRED_INSUFFICIENT = 2, /* image doesn't contain enough detail for recognition*/
    FINGERPRINT_ACQUIRED_IMAGER_DIRTY = 3, /* sensor needs to be cleaned */
    FINGERPRINT_ACQUIRED_TOO_SLOW = 4, /* mostly swipe-type sensors; not enough data collected */
    FINGERPRINT_ACQUIRED_TOO_FAST = 5, /* for swipe and area sensors; tell user to slow down*/
    FINGERPRINT_ACQUIRED_DETECTED = 6, /* when the finger is first detected. Used to optimize
                                          wakeup. Should be followed by one of the above messages */
    FINGERPRINT_ACQUIRED_VENDOR_BASE = 1000 /* vendor-specific acquisition messages start here */
} fingerprint_acquired_info_t;

typedef struct fingerprint_finger_id {
    uint32_t fid;
} fingerprint_finger_id_t;

typedef struct fingerprint_enroll {
    fingerprint_finger_id_t finger;
    /* samples_remaining goes from N (no data collected, but N scans needed)
     * to 0 (no more data is needed to build a template). */
    uint32_t samples_remaining;
    uint64_t msg; /* Vendor specific message. Used for user guidance */
} fingerprint_enroll_t;

typedef struct fingerprint_iterator {
    fingerprint_finger_id_t finger;
    uint32_t remaining_templates;
} fingerprint_iterator_t;

typedef fingerprint_iterator_t fingerprint_enumerated_t;
typedef fingerprint_iterator_t fingerprint_removed_t;

typedef struct fingerprint_acquired {
    fingerprint_acquired_info_t acquired_info; /* information about the image */
} fingerprint_acquired_t;

typedef struct fingerprint_authenticated {
    fingerprint_finger_id_t finger;
    hw_auth_token_t hat;
} fingerprint_authenticated_t;

typedef struct fingerprint_authenticator {
    uint64_t id;
} fingerprint_authenticator_t;

typedef struct fingerprint_challenge {
    uint64_t value;
} fingerprint_challenge_t;

typedef struct fingerprint_msg {
    fingerprint_msg_type_t type;
    union {
        fingerprint_error_t error;
        fingerprint_enroll_t enroll;
        fingerprint_enumerated_t enumerated;
        fingerprint_removed_t removed;
        fingerprint_acquired_t acquired;
        fingerprint_authenticated_t authenticated;
        fingerprint_authenticator_t authenticator;
        fingerprint_challenge_t challenge;
    } data;
} fingerprint_msg_t;

/* Callback function type */
typedef void (*fingerprint_notify_t)(const fingerprint_msg_t* msg);

/* Synchronous operation */
typedef struct fingerprint_device {
    /**
     * Common methods of the fingerprint device. This *must* be the first member
     * of fingerprint_device as users of this structure will cast a hw_device_t
     * to fingerprint_device pointer in contexts where it's known
     * the hw_device_t references a fingerprint_device.
     */
    struct hw_device_t common;

    /*
     * Client provided callback function to receive notifications.
     * Do not set by hand, use the function above instead.
     */
    fingerprint_notify_t notify;

    /*
     * Set notification callback:
     * Registers a user function that would receive notifications from the HAL
     * The call will block if the HAL state machine is in busy state until HAL
     * leaves the busy state.
     *
     * Function return: 0 if callback function is successfuly registered
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_notify)(struct fingerprint_device* dev, fingerprint_notify_t notify);

    /*
     * Fingerprint generate challenge:
     * Begins a secure transaction request. Note that the challenge by itself is not useful. It only
     * becomes useful when wrapped in a verifiable message such as a HardwareAuthToken.
     *
     * Notify with:
     *  message type: FINGERPRINT_CHALLENGE_GENERATED(7)
     *          data: uint64_t challenge
     */
    uint64_t (*generate_challenge)(struct fingerprint_device* dev);

    /*
     * Fingerprint revoke challenge:
     * Revokes a challenge that was previously generated. Note that if a non-existent challenge is
     * provided, the HAL must still notify the framework using ISessionCallback#onChallengeRevoked.
     *
     * Notify with:
     *  message type: FINGERPRINT_CHALLENGE_REVOKED(8)
     *          data: uint64_t challenge
     */
    int (*revoke_challenge)(struct fingerprint_device* dev, uint64_t challenge);

    /*
     * Fingerprint enroll request:
     * Switches the HAL state machine to collect and store a new fingerprint
     * template. Switches back as soon as enroll is complete
     * (fingerprint_msg.type == FINGERPRINT_TEMPLATE_ENROLLING &&
     *  fingerprint_msg.data.enroll.samples_remaining == 0)
     * or after timeout_sec seconds.
     * The fingerprint template will be assigned to the group gid. User has a choice
     * to supply the gid or set it to 0 in which case a unique group id will be generated.
     *
     * Function return: 0 if enrollment process can be successfully started
     *                  or a negative number in case of error, generally from the errno.h set.
     *                  A notify() function may be called indicating the error condition.
     */
    int (*enroll)(struct fingerprint_device* dev, const hw_auth_token_t* hat);

    /*
     * get_authenticator_id:
     * Returns a token associated with the current fingerprint set. This value will
     * change whenever a new fingerprint is enrolled, thus creating a new fingerprint
     * set.
     *
     * Function return: current authenticator id or 0 if function failed.
     */
    uint64_t (*get_authenticator_id)(struct fingerprint_device* dev);

    /*
     * invalidate_authenticator_id:
     * This operation only applies to sensors that are configured as SensorStrength::STRONG. If
     * invoked by the framework for sensors of other strengths, the HAL should immediately invoke
     * ISessionCallback#onAuthenticatorIdInvalidated.
     *
     * Notify with:
     *  message type: FINGERPRINT_AUTHENTICATOR_ID_INVALIDATED(10)
     *          data: long newAuthenticatorId
     */
    uint64_t (*invalidate_authenticator_id)(struct fingerprint_device* dev);

    /*
     * Cancel pending enroll or authenticate, sending FINGERPRINT_ERROR_CANCELED
     * to all running clients. Switches the HAL state machine back to the idle state.
     * Unlike enroll_done() doesn't invalidate the pre_enroll() challenge.
     *
     * Function return: 0 if cancel request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*cancel)(struct fingerprint_device* dev);

    /*
     * Enumerate all the fingerprint templates found in the directory set by
     * set_active_group()
     * For each template found a notify() will be called with:
     * fingerprint_msg.type == FINGERPRINT_TEMPLATE_ENUMERATED
     * fingerprint_msg.data.enumerated.finger indicating a template id
     * fingerprint_msg.data.enumerated.remaining_templates indicating how many more
     * enumeration messages to expect.
     * Note: If there are no fingerprints, then this should return 0 and the first fingerprint
     *                  enumerated should have fingerid=0 and remaining=0
     * Function return: 0 if enumerate request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*enumerate)(struct fingerprint_device* dev);

    /*
     * remove:
     * A request to remove the enrollments for this (sensorId, userId) pair.
     *
     * Notify with:
     *  message type: FINGERPRINT_TEMPLATE_REMOVED(4)
     *          data: { int enrollment,
     *                  int remaining_templates}
     *
     * @param fids a list of enrollments that should be removed.
     * @param count the count need to be removed
     */
    int (*remove)(struct fingerprint_device* dev, uint32_t* fids, uint32_t count);

    /*
     * Restricts the HAL operation to a set of fingerprints belonging to a
     * group provided.
     * The caller must provide a path to a storage location within the user's
     * data directory.
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_active_group)(struct fingerprint_device* dev, uint32_t userid,
                            const char* store_path);

    /*
     * Authenticates an operation identifed by operation_id
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*authenticate)(struct fingerprint_device* dev, uint64_t operation_id);

    /*
     * Clears the lockout counter after verifying the provided HAT (Hardware Auth Token).
     * If the HAT is invalid or expired, trigger an error via ISessionCallback#onError.
     * Lockout can also clear automatically after a timeout.
     * Notify with FINGERPRINT_RESET_LOCKOUT(11) and call ISessionCallback#onLockoutCleared when
     * done.
     */
    int (*reset_lockout)(struct fingerprint_device* dev, const hw_auth_token_t* hat);

    /*
     * onPointerDown:
     * This operation only applies to sensors that are configured as
     * FingerprintSensorType::UNDER_DISPLAY_*. If invoked erroneously by the framework for sensors
     * of other types, the HAL must treat this as a no-op and return immediately.
     *
     * @deprecated use onPointerDownWithContext instead.
     * Also empty in Xiaomi's fingerprint module
     */
    void (*onPointerDown)(struct fingerprint_device* dev, int32_t pointerId, int32_t x, int32_t y,
                          float minor, float major);

    /*
     * onPointerUp:
     * This operation only applies to sensors that are configured as
     * FingerprintSensorType::UNDER_DISPLAY_*. If invoked for sensors of other types, the HAL must
     * treat this as a no-op and return immediately.
     *
     * @deprecated use onPointerUpWithContext instead.
     * Also empty in Xiaomi's fingerprint module
     */
    void (*onPointerUp)(struct fingerprint_device* dev, int32_t pointerId);

    /*
     * Xiaomi fingerprint extension command.
     */
    int (*extCmd)(struct fingerprint_device* dev, int32_t cmd, int32_t param);

    /* Reserved for backward binary compatibility */
    void* reserved[4];
} fingerprint_device_t;

typedef struct fingerprint_module {
    /**
     * Common methods of the fingerprint module. This *must* be the first member
     * of fingerprint_module as users of this structure will cast a hw_module_t
     * to fingerprint_module pointer in contexts where it's known
     * the hw_module_t references a fingerprint_module.
     */
    struct hw_module_t common;
} fingerprint_module_t;
