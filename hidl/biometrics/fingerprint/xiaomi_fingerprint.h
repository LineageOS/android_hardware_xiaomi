/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <hardware/fingerprint.h>

typedef struct xiaomi_fingerprint_device {
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
    int (*set_notify)(struct xiaomi_fingerprint_device* dev, fingerprint_notify_t notify);

    /*
     * Fingerprint pre-enroll enroll request:
     * Generates a unique token to upper layers to indicate the start of an enrollment transaction.
     * This token will be wrapped by security for verification and passed to enroll() for
     * verification before enrollment will be allowed. This is to ensure adding a new fingerprint
     * template was preceded by some kind of credential confirmation (e.g. device password).
     *
     * Function return: 0 if function failed
     *                  otherwise, a uint64_t of token
     */
    uint64_t (*pre_enroll)(struct xiaomi_fingerprint_device* dev);

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
    int (*enroll)(struct xiaomi_fingerprint_device* dev, const hw_auth_token_t* hat, uint32_t gid,
                  uint32_t timeout_sec);

    /*
     * Finishes the enroll operation and invalidates the pre_enroll() generated challenge.
     * This will be called at the end of a multi-finger enrollment session to indicate
     * that no more fingers will be added.
     *
     * Function return: 0 if the request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*post_enroll)(struct xiaomi_fingerprint_device* dev);

    /*
     * get_authenticator_id:
     * Returns a token associated with the current fingerprint set. This value will
     * change whenever a new fingerprint is enrolled, thus creating a new fingerprint
     * set.
     *
     * Function return: current authenticator id or 0 if function failed.
     */
    uint64_t (*get_authenticator_id)(struct xiaomi_fingerprint_device* dev);

    /*
     * Cancel pending enroll or authenticate, sending FINGERPRINT_ERROR_CANCELED
     * to all running clients. Switches the HAL state machine back to the idle state.
     * Unlike enroll_done() doesn't invalidate the pre_enroll() challenge.
     *
     * Function return: 0 if cancel request is accepted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*cancel)(struct xiaomi_fingerprint_device* dev);

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
    int (*enumerate)(struct xiaomi_fingerprint_device* dev);

    /*
     * Fingerprint remove request:
     * Deletes a fingerprint template.
     * Works only within the path set by set_active_group().
     * The fid parameter can be used as a widcard:
     *   * fid == 0 -- delete all the templates in the group.
     *   * fid != 0 -- delete this specific template from the group.
     * For each template found a notify() will be called with:
     * fingerprint_msg.type == FINGERPRINT_TEMPLATE_REMOVED
     * fingerprint_msg.data.removed.finger indicating a template id deleted
     * fingerprint_msg.data.removed.remaining_templates indicating how many more
     * templates will be deleted by this operation.
     *
     * Function return: 0 if fingerprint template(s) can be successfully deleted
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*remove)(struct xiaomi_fingerprint_device* dev, uint32_t gid, uint32_t fid);

    /*
     * Restricts the HAL operation to a set of fingerprints belonging to a
     * group provided.
     * The caller must provide a path to a storage location within the user's
     * data directory.
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*set_active_group)(struct xiaomi_fingerprint_device* dev, uint32_t gid,
                            const char* store_path);

    /*
     * Authenticates an operation identifed by operation_id
     *
     * Function return: 0 on success
     *                  or a negative number in case of error, generally from the errno.h set.
     */
    int (*authenticate)(struct xiaomi_fingerprint_device* dev, uint64_t operation_id, uint32_t gid);

    /*
     * Call a Xiaomi fingerprint extension command.
     */
    int (*extCmd)(struct xiaomi_fingerprint_device* dev, int32_t cmd, int32_t param);

    /*
     * Reserved for backward binary compatibility
     */
    void* reserved[4];
} xiaomi_fingerprint_device_t;
