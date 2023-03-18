/*
 * SPDX-FileCopyrightText: 2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.hardware.Sensor
import android.hardware.SensorManager

fun SensorManager.getSensor(type: String) =
    getSensorList(Sensor.TYPE_ALL).firstOrNull { type == it.stringType }
