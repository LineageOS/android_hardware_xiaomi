/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.fx.tunnel;

import vendor.xiaomi.hardware.fx.tunnel.IMiFxTunnelCallback;
import vendor.xiaomi.hardware.fx.tunnel.IMiFxTunnelCommandResult;

@VintfStability
interface IMiFxTunnel {
    IMiFxTunnelCommandResult invokeCommand(int cmdId, in byte[] param);
    void setNotify(in IMiFxTunnelCallback iMiFxTunnelCallback);
}
