/*
 * SPDX-FileCopyrightText: 2023-2025 Paranoid Android
 * SPDX-License-Identifier: Apache-2.0
 */

package com.xiaomi.mtb

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log
import com.xiaomi.mtb.EsimController

class BootCompletedReceiver : BroadcastReceiver() {

    companion object {
        private const val TAG = "EsimSwitcher"
        private val DEBUG = Log.isLoggable(TAG, Log.DEBUG)
    }

    override fun onReceive(context: Context, intent: Intent) {
        if (DEBUG) Log.d(TAG, "Received boot completed intent: ${intent.action}")
        if (intent.action == Intent.ACTION_BOOT_COMPLETED) {
            EsimController.getInstance(context).onBootCompleted()
        }
    }
}
