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
import android.util.Log
import org.lineageos.settings.doze.DozeUtils.isHandwaveGestureEnabled
import org.lineageos.settings.doze.DozeUtils.isPocketGestureEnabled
import org.lineageos.settings.doze.DozeUtils.launchDozePulse
import java.util.concurrent.Executors

class ProximitySensor(private val context: Context) : SensorEventListener {
    var sawNear = false
        private set
    private val sensorManager = context.getSystemService(SensorManager::class.java)
    private val sensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY, false)
    private val executorService = Executors.newSingleThreadExecutor()
    private var inPocketTime = 0L

    override fun onSensorChanged(event: SensorEvent) {
        val isNear = event.values[0] < sensor.maximumRange

        if (sawNear && !isNear) {
            if (shouldPulse(event.timestamp)) {
                launchDozePulse(context)
            }
        } else {
            inPocketTime = event.timestamp
        }

        sawNear = isNear
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {}

    fun enable() {
        Log.d(LOG_TAG, "Enabling")

        submit {
            sensorManager.registerListener(
                this, sensor,
                SensorManager.SENSOR_DELAY_NORMAL
            )
        }
    }

    fun disable() {
        Log.d(LOG_TAG, "Disabling")

        submit { sensorManager.unregisterListener(this, sensor) }
    }

    private fun submit(runnable: Runnable) = executorService.submit(runnable)

    private fun shouldPulse(timestamp: Long): Boolean {
        val delta = timestamp - inPocketTime

        return if (isHandwaveGestureEnabled(context) && isPocketGestureEnabled(context)) {
            true
        } else if (isHandwaveGestureEnabled(context)) {
            delta < HANDWAVE_MAX_DELTA_NS
        } else if (isPocketGestureEnabled(context)) {
            delta >= POCKET_MIN_DELTA_NS
        } else {
            false
        }
    }

    companion object {
        private const val LOG_TAG = "ProximitySensor"

        // Maximum time for the hand to cover the sensor: 1s
        private const val HANDWAVE_MAX_DELTA_NS = 1000 * 1000 * 1000

        // Minimum time until the device is considered to have been in the pocket: 2s
        private const val POCKET_MIN_DELTA_NS = 2000 * 1000 * 1000
    }
}
