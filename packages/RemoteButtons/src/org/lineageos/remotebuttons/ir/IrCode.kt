/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.ir

/** An IR code already encoded for a particular remote family. */
sealed interface IrCode {
    /** Only programmers that know [format] accept this code. */
    data class Raw(val format: String, val data: String) : IrCode
}
