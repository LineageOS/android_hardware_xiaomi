/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.tencent.soter.soterserver;

public final class SoterErrorCode {
    public static final int SOTER_ERROR_OK = 0;
    public static final int SOTER_ERROR_ATTK_IS_VALID = -1;
    public static final int SOTER_ERROR_ATTK_NOT_EXIST = -2;
    public static final int SOTER_ERROR_ATTK_DIGEST_NOT_MATCH = -3;
    public static final int SOTER_ERROR_ATTK_DIGEST_NOT_READY = -4;
    public static final int SOTER_ERROR_ASK_NOT_READY = -5;
    public static final int SOTER_ERROR_AUTH_KEY_NOT_READY = -6;
    public static final int SOTER_ERROR_SESSION_OUT_OF_TIME = -7;
    public static final int SOTER_ERROR_NO_AUTH_KEY_MATCHED = -8;
    public static final int SOTER_ERROR_IS_AUTHING = -9;
    public static final int SOTER_ERROR_OTHERS = -10;
    public static final int SOTER_ERROR_MEMORY_ALLOCATION_FAILED = -11;
    public static final int SOTER_ERROR_SOTER_NOT_ENABLED = -12;
    public static final int SOTER_ERROR_ATTK_NOT_PROVISIONED = -13;
    public static final int SOTER_SECURITY_STATE_FAILURE = -14;
    public static final int SOTER_ERROR_INVALID_TAG = -15;
    public static final int SOTER_ERROR_INVALID_ARGUMENT = -16;
    public static final int SOTER_ERROR_UNSUPPORTED_KEY_SIZE = -17;
    public static final int SOTER_ERROR_SECURE_HW_COMMUNICATION_FAILED = -18;
    public static final int SOTER_ERROR_ATTK_ALREADY_PROVISIONED = -20;
    public static final int SOTER_RPMB_NOT_PROVISIONED = -21;
    public static final int SOTER_ERROR_INSUFFICIENT_BUFFER_SPACE = -22;
    public static final int SOTER_ERROR_UNSUPPORTED_DIGEST = -23;
    public static final int SOTER_ERROR_UNSUPPORTED_PADDING_MODE = -24;
    public static final int SOTER_ERROR_INVALID_KEY_BLOB = -25;
    public static final int SOTER_ERROR_VERIFICATION_FAILED = -26;
    public static final int SOTER_ERROR_INVALID_AUTHORIZATION_TIMEOUT = -27;
    public static final int SOTER_ERROR_KEY_EXPORT_OPTIONS_INVALID = -28;
    public static final int SOTER_ERROR_UNEXPECTED_NULL_POINTER = -29;

    public static final int SOTER_WRAPPERERROR_UNKNOWN = -200;
    public static final int SOTER_ERROR_UID_NULL = -201;
    public static final int SOTER_ERROR_KNAME_NULL = -202;
    public static final int SOTER_ERROR_CHALLENGE_NULL = -203;
    public static final int SOTER_ERROR_OPERATEID_NULL = -204;
    public static final int SOTER_ERROR_UNKNOWN_ERROR = -1000;

    public static final int ERR_OK = SOTER_ERROR_OK;
    public static final int ERR_UNKNOWN = SOTER_WRAPPERERROR_UNKNOWN;
    public static final int ERR_FAILED = SOTER_ERROR_UNKNOWN_ERROR;

    public static String toString(int o) {
        if (o == SOTER_ERROR_OK) {
            return "SOTER_ERROR_OK";
        }
        if (o == SOTER_ERROR_ATTK_IS_VALID) {
            return "SOTER_ERROR_ATTK_IS_VALID";
        }
        if (o == SOTER_ERROR_ATTK_NOT_EXIST) {
            return "SOTER_ERROR_ATTK_NOT_EXIST";
        }
        if (o == SOTER_ERROR_ATTK_DIGEST_NOT_MATCH) {
            return "SOTER_ERROR_ATTK_DIGEST_NOT_MATCH";
        }
        if (o == SOTER_ERROR_ATTK_DIGEST_NOT_READY) {
            return "SOTER_ERROR_ATTK_DIGEST_NOT_READY";
        }
        if (o == SOTER_ERROR_ASK_NOT_READY) {
            return "SOTER_ERROR_ASK_NOT_READY";
        }
        if (o == SOTER_ERROR_AUTH_KEY_NOT_READY) {
            return "SOTER_ERROR_AUTH_KEY_NOT_READY";
        }
        if (o == SOTER_ERROR_SESSION_OUT_OF_TIME) {
            return "SOTER_ERROR_SESSION_OUT_OF_TIME";
        }
        if (o == SOTER_ERROR_NO_AUTH_KEY_MATCHED) {
            return "SOTER_ERROR_NO_AUTH_KEY_MATCHED";
        }
        if (o == SOTER_ERROR_IS_AUTHING) {
            return "SOTER_ERROR_IS_AUTHING";
        }
        if (o == SOTER_ERROR_OTHERS) {
            return "SOTER_ERROR_OTHERS";
        }
        if (o == SOTER_ERROR_MEMORY_ALLOCATION_FAILED) {
            return "SOTER_ERROR_MEMORY_ALLOCATION_FAILED";
        }
        if (o == SOTER_ERROR_SOTER_NOT_ENABLED) {
            return "SOTER_ERROR_SOTER_NOT_ENABLED";
        }
        if (o == SOTER_ERROR_ATTK_NOT_PROVISIONED) {
            return "SOTER_ERROR_ATTK_NOT_PROVISIONED";
        }
        if (o == SOTER_SECURITY_STATE_FAILURE) {
            return "SOTER_SECURITY_STATE_FAILURE";
        }
        if (o == SOTER_ERROR_INVALID_TAG) {
            return "SOTER_ERROR_INVALID_TAG";
        }
        if (o == SOTER_ERROR_INVALID_ARGUMENT) {
            return "SOTER_ERROR_INVALID_ARGUMENT";
        }
        if (o == SOTER_ERROR_UNSUPPORTED_KEY_SIZE) {
            return "SOTER_ERROR_UNSUPPORTED_KEY_SIZE";
        }
        if (o == SOTER_ERROR_SECURE_HW_COMMUNICATION_FAILED) {
            return "SOTER_ERROR_SECURE_HW_COMMUNICATION_FAILED";
        }
        if (o == SOTER_ERROR_ATTK_ALREADY_PROVISIONED) {
            return "SOTER_ERROR_ATTK_ALREADY_PROVISIONED";
        }
        if (o == SOTER_RPMB_NOT_PROVISIONED) {
            return "SOTER_RPMB_NOT_PROVISIONED";
        }
        if (o == SOTER_ERROR_INSUFFICIENT_BUFFER_SPACE) {
            return "SOTER_ERROR_INSUFFICIENT_BUFFER_SPACE";
        }
        if (o == SOTER_ERROR_UNSUPPORTED_DIGEST) {
            return "SOTER_ERROR_UNSUPPORTED_DIGEST";
        }
        if (o == SOTER_ERROR_UNSUPPORTED_PADDING_MODE) {
            return "SOTER_ERROR_UNSUPPORTED_PADDING_MODE";
        }
        if (o == SOTER_ERROR_INVALID_KEY_BLOB) {
            return "SOTER_ERROR_INVALID_KEY_BLOB";
        }
        if (o == SOTER_ERROR_VERIFICATION_FAILED) {
            return "SOTER_ERROR_VERIFICATION_FAILED";
        }
        if (o == SOTER_ERROR_INVALID_AUTHORIZATION_TIMEOUT) {
            return "SOTER_ERROR_INVALID_AUTHORIZATION_TIMEOUT";
        }
        if (o == SOTER_ERROR_KEY_EXPORT_OPTIONS_INVALID) {
            return "SOTER_ERROR_KEY_EXPORT_OPTIONS_INVALID";
        }
        if (o == SOTER_ERROR_UNEXPECTED_NULL_POINTER) {
            return "SOTER_ERROR_UNEXPECTED_NULL_POINTER";
        }
        if (o == SOTER_WRAPPERERROR_UNKNOWN) {
            return "SOTER_WRAPPERERROR_UNKNOWN";
        }
        if (o == SOTER_ERROR_UID_NULL) {
            return "SOTER_ERROR_UID_NULL";
        }
        if (o == SOTER_ERROR_KNAME_NULL) {
            return "SOTER_ERROR_KNAME_NULL";
        }
        if (o == SOTER_ERROR_CHALLENGE_NULL) {
            return "SOTER_ERROR_CHALLENGE_NULL";
        }
        if (o == SOTER_ERROR_OPERATEID_NULL) {
            return "SOTER_ERROR_OPERATEID_NULL";
        }
        if (o == SOTER_ERROR_UNKNOWN_ERROR) {
            return "SOTER_ERROR_UNKNOWN_ERROR";
        }
        return "0x" + Integer.toHexString(o);
    }
}
