/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotevoice

import android.content.Context
import android.media.AudioAttributes
import android.media.AudioDeviceInfo
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioRecordingConfiguration
import android.media.AudioTrack
import android.media.MediaRecorder
import android.media.audiopolicy.AudioMix
import android.media.audiopolicy.AudioMixingRule
import android.media.audiopolicy.AudioPolicy
import android.os.Handler
import android.util.Log

/**
 * Routes app captures to a remote submix that we feed, as if the remote were the device's
 * microphone. Hotword captures are left alone, so the remote is never kept streaming in the
 * background.
 */
class MicInjector(
    private val context: Context,
    private val handler: Handler,
    private val onCaptureChanged: (active: Boolean) -> Unit,
) {
    private val audioManager = context.getSystemService(AudioManager::class.java)!!

    private val mix =
        AudioMix.Builder(
                AudioMixingRule.Builder()
                    .setTargetMixRole(AudioMixingRule.MIX_ROLE_INJECTOR)
                    .apply {
                        CAPTURE_PRESETS.forEach {
                            addMixRule(
                                AudioMixingRule.RULE_MATCH_ATTRIBUTE_CAPTURE_PRESET,
                                AudioAttributes.Builder().setCapturePreset(it).build(),
                            )
                        }
                    }
                    .build()
            )
            .setFormat(
                AudioFormat.Builder()
                    .setSampleRate(MsbcDecoder.SAMPLE_RATE)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                    .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                    .build()
            )
            .setRouteFlags(AudioMix.ROUTE_FLAG_LOOP_BACK)
            // Keep our submix input connected while the remote is idle, otherwise captures only
            // reach it once the injector is playing
            .setPersistent(true)
            .build()

    private var active = false

    /**
     * Our mix also reports the activity of our own track, so tell captures apart by watching the
     * recordings that reach the remote submix instead.
     */
    private val recordingCallback =
        object : AudioManager.AudioRecordingCallback() {
            override fun onRecordingConfigChanged(configs: List<AudioRecordingConfiguration>) {
                val nowActive =
                    configs.any {
                        it.audioDevice?.type == AudioDeviceInfo.TYPE_REMOTE_SUBMIX &&
                            // Rather than an app capturing the submix on purpose
                            it.clientAudioSource != MediaRecorder.AudioSource.REMOTE_SUBMIX &&
                            !it.isClientSilenced
                    }
                if (nowActive == active) return
                active = nowActive
                Log.d(TAG, "capture ${if (active) "started" else "stopped"}")
                if (active) {
                    track?.play()
                } else {
                    track?.pause()
                    track?.flush()
                }
                onCaptureChanged(active)
            }
        }

    private val policy = AudioPolicy.Builder(context).addMix(mix).setLooper(handler.looper).build()

    private var track: AudioTrack? = null

    fun register(): Boolean {
        if (audioManager.registerAudioPolicy(policy) != AudioManager.SUCCESS) {
            Log.e(TAG, "registerAudioPolicy failed")
            return false
        }
        track = policy.createAudioTrackSource(mix)
        audioManager.registerAudioRecordingCallback(recordingCallback, handler)
        recordingCallback.onRecordingConfigChanged(audioManager.activeRecordingConfigurations)
        Log.i(TAG, "remote microphone available")
        return true
    }

    fun unregister() {
        audioManager.unregisterAudioRecordingCallback(recordingCallback)
        track?.release()
        track = null
        audioManager.unregisterAudioPolicy(policy)
        Log.i(TAG, "remote microphone gone")
    }

    /**
     * Feeds decoded samples to the capturing apps. Audio arrives in real time but in bursts, which
     * can briefly fill the track buffer, so this may wait a few milliseconds.
     */
    fun write(samples: ShortArray, count: Int) {
        if (!active) return
        track?.write(samples, 0, count, AudioTrack.WRITE_BLOCKING)
    }

    companion object {
        private const val TAG = "MicInjector"

        private val CAPTURE_PRESETS =
            intArrayOf(
                MediaRecorder.AudioSource.MIC,
                MediaRecorder.AudioSource.CAMCORDER,
                MediaRecorder.AudioSource.VOICE_RECOGNITION,
                MediaRecorder.AudioSource.VOICE_COMMUNICATION,
                MediaRecorder.AudioSource.UNPROCESSED,
                MediaRecorder.AudioSource.VOICE_PERFORMANCE,
            )
    }
}
