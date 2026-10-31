/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.ir

/**
 * The codes for one model family of a brand.
 *
 * @param id Stable identifier, unique within its database.
 * @param keys The codes for the programmable buttons; a code set always has volume up and down.
 * @param rawParameters For [IrCode.Raw] codes, the protocol parameters shared by all keys.
 */
data class CodeSet(
    val id: String,
    val keys: Map<IrKey, IrCode>,
    val rawParameters: String? = null,
)
