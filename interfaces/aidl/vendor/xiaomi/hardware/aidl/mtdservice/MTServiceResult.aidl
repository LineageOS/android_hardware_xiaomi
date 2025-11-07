/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aidl.mtdservice;

@VintfStability
parcelable MTServiceResult {
    int ret = 0;
    byte[] rbuf;
}
