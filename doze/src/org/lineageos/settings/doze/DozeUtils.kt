/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.Context
import android.content.Intent
import android.hardware.display.AmbientDisplayConfiguration
import android.os.UserHandle
import android.provider.Settings
import android.provider.Settings.Secure.DOZE_ALWAYS_ON
import android.provider.Settings.Secure.DOZE_ENABLED
import android.util.Log

import androidx.preference.PreferenceManager

object DozeUtils {
    private const val LOG_TAG = "DozeUtils"

    internal const val DOZE_ENABLE = "doze_enable"

    internal const val ALWAYS_ON_DISPLAY = "always_on_display"

    internal const val CATEG_PICKUP_SENSOR = "pickup_sensor"
    internal const val CATEG_PROX_SENSOR = "proximity_sensor"

    internal const val GESTURE_PICK_UP_KEY = "gesture_pick_up"
    internal const val GESTURE_HAND_WAVE_KEY = "gesture_hand_wave"
    internal const val GESTURE_POCKET_KEY = "gesture_pocket"

    private const val DOZE_INTENT = "com.android.systemui.doze.pulse"

    internal fun enableDoze(context: Context, enable: Boolean) = Settings.Secure.putInt(
        context.contentResolver,
        DOZE_ENABLED, if (enable) 1 else 0
    )

    fun isDozeEnabled(context: Context) = Settings.Secure.getInt(
        context.contentResolver,
        DOZE_ENABLED, 1
    ) != 0

    fun launchDozePulse(context: Context) {
        Log.d(LOG_TAG, "Launch doze pulse")

        context.sendBroadcastAsUser(
            Intent(DOZE_INTENT),
            UserHandle(UserHandle.USER_CURRENT)
        )
    }

    internal fun enableAlwaysOn(context: Context, enable: Boolean) =
        Settings.Secure.putIntForUser(
            context.contentResolver,
            DOZE_ALWAYS_ON, if (enable) 1 else 0, UserHandle.USER_CURRENT
        )

    internal fun isAlwaysOnEnabled(context: Context): Boolean {
        val enabledByDefault = context.resources
            .getBoolean(com.android.internal.R.bool.config_dozeAlwaysOnEnabled)
        return Settings.Secure.getIntForUser(
            context.contentResolver,
            DOZE_ALWAYS_ON, if (alwaysOnDisplayAvailable(context) && enabledByDefault) 1 else 0,
            UserHandle.USER_CURRENT
        ) != 0
    }

    internal fun alwaysOnDisplayAvailable(context: Context) =
        AmbientDisplayConfiguration(context).alwaysOnAvailable()

    private fun isGestureEnabled(context: Context, gesture: String?) =
        PreferenceManager.getDefaultSharedPreferences(context)
            .getBoolean(gesture, false)

    fun isPickUpEnabled(context: Context) = isGestureEnabled(context, GESTURE_PICK_UP_KEY)
    fun isHandwaveGestureEnabled(context: Context) =
        isGestureEnabled(context, GESTURE_HAND_WAVE_KEY)
    fun isPocketGestureEnabled(context: Context) = isGestureEnabled(context, GESTURE_POCKET_KEY)

    fun sensorsEnabled(context: Context) =
        (isPickUpEnabled(context)
                || isHandwaveGestureEnabled(context)
                || isPocketGestureEnabled(context))
}
