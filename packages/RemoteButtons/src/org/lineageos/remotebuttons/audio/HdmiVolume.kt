/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.audio

import android.content.Context
import android.media.AudioDeviceAttributes
import android.media.AudioDeviceInfo
import android.media.AudioDeviceVolumeManager
import android.util.Log

/**
 * Tells the system who sets the volume on HDMI. Once the remote's volume buttons control the TV
 * over IR, the box outputs at full volume, which also lets the user choose surround formats:
 * TvSettings disables them while HDMI has a variable volume.
 */
object HdmiVolume {
    private const val TAG = "HdmiVolume"

    private val HDMI =
        AudioDeviceAttributes(AudioDeviceAttributes.ROLE_OUTPUT, AudioDeviceInfo.TYPE_HDMI, "")

    fun setControlledByTv(context: Context, controlledByTv: Boolean) {
        val manager = context.getSystemService(AudioDeviceVolumeManager::class.java) ?: return
        val behavior =
            if (controlledByTv) {
                AudioDeviceVolumeManager.DEVICE_VOLUME_BEHAVIOR_FULL
            } else {
                AudioDeviceVolumeManager.DEVICE_VOLUME_BEHAVIOR_VARIABLE
            }
        try {
            manager.setDeviceVolumeBehavior(HDMI, behavior)
        } catch (e: SecurityException) {
            Log.e(TAG, "Failed to set the HDMI volume behavior", e)
        }
    }
}
