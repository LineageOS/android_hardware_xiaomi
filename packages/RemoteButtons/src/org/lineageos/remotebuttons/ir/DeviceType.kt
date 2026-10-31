/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.ir

/** What the remote should control. */
enum class DeviceType(val id: String) {
    TV("tv"),
    SOUNDBAR("soundbar"),
    RECEIVER("receiver"),
}
