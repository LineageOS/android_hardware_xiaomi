/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aidl.tidaservice;

@VintfStability
interface ITidaService {
    boolean contains(int i, String str);
    int generateKeyPair(int i, String str, String str2);
    String getFpIds();
    int removeAllKey(int i1);
    byte[] sign();
    int signInit(int i, String str, String str2);
    int signUpdate(int i, String str);
}
