/*
 * Copyright (C) 2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.euicc

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log

class EuiccReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        Log.d(TAG, "Received PARTNER_CUSTOMIZATION intent")
    }

    companion object {
        private const val TAG = "XiaomiEuiccReceiver"
    }
}
