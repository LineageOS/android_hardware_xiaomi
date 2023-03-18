/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.IBinder
import android.os.UserHandle
import android.util.Log
import org.lineageos.settings.doze.DozeUtils.isHandwaveGestureEnabled
import org.lineageos.settings.doze.DozeUtils.isPickUpEnabled
import org.lineageos.settings.doze.DozeUtils.isPocketGestureEnabled

class DozeService : Service() {
    private lateinit var proximitySensor: ProximitySensor
    private lateinit var pickupSensor: PickupSensor

    private val screenStateReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            when (intent.action) {
                Intent.ACTION_SCREEN_ON -> onDisplayOn()
                Intent.ACTION_SCREEN_OFF -> onDisplayOff()
            }
        }
    }

    override fun onCreate() {
        Log.d(LOG_TAG, "Creating service")

        proximitySensor = ProximitySensor(this)
        pickupSensor = PickupSensor(this)

        registerReceiver(screenStateReceiver, IntentFilter().apply {
            addAction(Intent.ACTION_SCREEN_ON)
            addAction(Intent.ACTION_SCREEN_OFF)
        })
    }

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        Log.d(LOG_TAG, "Starting service")

        return START_STICKY
    }

    override fun onDestroy() {
        Log.d(LOG_TAG, "Destroying service")

        unregisterReceiver(screenStateReceiver)
        proximitySensor.disable()
        pickupSensor.disable()

        super.onDestroy()
    }

    override fun onBind(intent: Intent): IBinder? = null

    private fun onDisplayOn() {
        Log.d(LOG_TAG, "Display on")

        if (isPickUpEnabled(this)) {
            pickupSensor.disable()
        }
        if (isHandwaveGestureEnabled(this) ||
            isPocketGestureEnabled(this)
        ) {
            proximitySensor.disable()
        }
    }

    private fun onDisplayOff() {
        Log.d(LOG_TAG, "Display off")

        if (isPickUpEnabled(this)) {
            pickupSensor.enable()
        }
        if (isHandwaveGestureEnabled(this) ||
            isPocketGestureEnabled(this)
        ) {
            proximitySensor.enable()
        }
    }

    companion object {
        private const val LOG_TAG = "DozeService"

        private fun startService(context: Context) {
            Log.d(LOG_TAG, "Starting service")

            context.startServiceAsUser(
                Intent(context, DozeService::class.java),
                UserHandle.CURRENT
            )
        }

        private fun stopService(context: Context) {
            Log.d(LOG_TAG, "Stopping service")

            context.stopServiceAsUser(
                Intent(context, DozeService::class.java),
                UserHandle.CURRENT
            )
        }

        fun checkService(context: Context) {
            if (DozeUtils.isDozeEnabled(context)
                && !DozeUtils.isAlwaysOnEnabled(context)
                && DozeUtils.sensorsEnabled(context)
            ) {
                startService(context)
            } else {
                stopService(context)
            }
        }
    }
}
