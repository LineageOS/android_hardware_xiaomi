/*
 * SPDX-FileCopyrightText: 2015 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

class BootCompletedReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        assert(intent.action == Intent.ACTION_BOOT_COMPLETED)

        Log.d(LOG_TAG, "Received boot completed intent")

        DozeService.checkService(context)
    }

    companion object {
        private const val LOG_TAG = "XiaomiDoze"
    }
}
