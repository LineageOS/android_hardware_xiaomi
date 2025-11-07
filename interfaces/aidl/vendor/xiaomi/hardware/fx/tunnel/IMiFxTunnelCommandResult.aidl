/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.fx.tunnel;

@VintfStability
parcelable IMiFxTunnelCommandResult {
    int errCode = 0;
    byte[] data;
}
