/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.SystemClock
import android.util.Log

import org.lineageos.settings.doze.DozeUtils.launchDozePulse

import java.util.concurrent.Executors
import java.util.concurrent.Future

class PickupSensor(private val context: Context) : SensorEventListener {
    private val sensorManager = context.getSystemService(SensorManager::class.java)
    private val sensor = sensorManager.getSensor(SENSOR_NAME)
    private val executorService = Executors.newSingleThreadExecutor()
    private var entryTimestamp = 0L

    override fun onSensorChanged(event: SensorEvent) {
        if (DEBUG) Log.d(LOG_TAG, "Got sensor event: ${event.values[0]}")

        val delta = SystemClock.elapsedRealtime() - entryTimestamp
        if (delta < MIN_PULSE_INTERVAL_MS) {
            return
        }

        entryTimestamp = SystemClock.elapsedRealtime()
        if (event.values[0] == 1f) {
            launchDozePulse(context)
        }
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {}

    fun enable() {
        Log.d(LOG_TAG, "Enabling")

        submit {
            sensorManager.registerListener(
                this, sensor,
                SensorManager.SENSOR_DELAY_NORMAL
            )
            entryTimestamp = SystemClock.elapsedRealtime()
        }
    }

    fun disable() {
        Log.d(LOG_TAG, "Disabling")

        submit { sensorManager.unregisterListener(this, sensor) }
    }

    private fun submit(runnable: Runnable): Future<*> {
        return executorService.submit(runnable)
    }

    companion object {
        private const val LOG_TAG = "PickupSensor"
        private const val DEBUG = false

        private const val SENSOR_NAME = "xiaomi.sensor.pickup"

        private const val MIN_PULSE_INTERVAL_MS = 2500
    }
}
