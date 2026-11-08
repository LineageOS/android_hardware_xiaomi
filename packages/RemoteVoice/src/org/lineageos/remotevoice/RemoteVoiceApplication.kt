/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotevoice

import android.Manifest
import android.app.Application
import android.content.pm.PackageManager
import android.util.Log

/** Persistent process that serves the microphone of paired Xiaomi remotes to the system. */
class RemoteVoiceApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        // Being persistent, we are restarted right away if we crash, so don't
        if (
            checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) !=
                PackageManager.PERMISSION_GRANTED
        ) {
            Log.e(TAG, "BLUETOOTH_CONNECT not granted, remote microphones are unavailable")
            return
        }
        RemoteVoiceManager(this).start()
    }

    companion object {
        private const val TAG = "RemoteVoice"
    }
}
