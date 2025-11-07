/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.mfidoca;

@VintfStability
interface IFidoService {
    byte[] execute(in byte[] bArr, int i);
    int fido_key_extract(in byte[] bArr, int i);
    int fido_key_get_version();
    int fido_key_load(String str, String str2);
    String fido_key_prepar();
}
