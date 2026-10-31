/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.ir

/** Remote buttons that can be programmed to send IR. */
enum class IrKey(val id: String) {
    POWER("power"),
    VOLUME_UP("vol_up"),
    VOLUME_DOWN("vol_down");

    companion object {
        fun fromId(id: String) = entries.firstOrNull { it.id == id }
    }
}
