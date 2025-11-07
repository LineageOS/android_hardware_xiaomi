/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.fx.tunnel;

@VintfStability
interface IMiFxTunnelCallback {
    void onDaemonMessage(long devId, int msgId, int cmdId, in byte[] data);
}
