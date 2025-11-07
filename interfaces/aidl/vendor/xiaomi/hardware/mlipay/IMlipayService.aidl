/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.mlipay;

@VintfStability
interface IMlipayService {
    int[] ifaa_get_idlist(int i);
    String ifaa_key_dump();
    int ifaa_key_get_version();
    int ifaa_key_load(String str, String str2);
    String ifaa_key_prepare();
    byte[] invoke_command(in byte[] bArr, int i);
}
