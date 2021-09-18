/*
 * SPDX-FileCopyrightText: 2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.Context

val Context.pickupSensorType
    get() = getString(R.string.config_pickupSensorType).takeUnless {
        it.isEmpty()
    }

val Context.proximitySensorType
    get() = getString(R.string.config_proximitySensorType).takeUnless {
        it.isEmpty()
    }
