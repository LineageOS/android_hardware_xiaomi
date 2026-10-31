/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.database

import org.lineageos.remotebuttons.ir.CodeSet
import org.lineageos.remotebuttons.ir.DeviceType

data class Brand(val id: String, val name: String)

/** A source of IR code sets, organised by device type and brand. */
interface IrDatabase {
    /** Brands with at least one code set for [type], alphabetically. */
    suspend fun brands(type: DeviceType): List<Brand>

    /** The most common brands for [type], most common first. Empty when unknown. */
    suspend fun popularBrands(type: DeviceType): List<Brand> = emptyList()

    /** Code sets to try for [brand], most likely first. */
    suspend fun codeSets(type: DeviceType, brand: Brand): List<CodeSet>
}
