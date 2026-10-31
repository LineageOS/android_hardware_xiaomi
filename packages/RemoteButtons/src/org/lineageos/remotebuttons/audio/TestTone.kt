/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.audio

import android.content.Context
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioTrack
import kotlin.math.PI
import kotlin.math.roundToInt
import kotlin.math.sin

/**
 * Looping chime played while the user checks whether the remote's volume buttons reach the
 * TV. While it plays the music volume is set to a fixed middle level, then restored.
 */
class TestTone(context: Context) {

    private val audioManager = context.getSystemService(AudioManager::class.java)
    private var track: AudioTrack? = null
    private var savedVolume: Int? = null

    fun start() {
        if (track != null) return
        savedVolume = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC)
        val max = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC)
        audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, (max * VOLUME_FRACTION).roundToInt(), 0)
        track = chime().apply { play() }
    }

    fun stop() {
        track?.run {
            stop()
            release()
        }
        track = null
        savedVolume?.let { audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, it, 0) }
        savedVolume = null
    }

    /** A soft C major arpeggio. */
    private fun chime(): AudioTrack {
        val noteLength = SAMPLE_RATE * 2 / 5
        val pcm = ShortArray(noteLength * NOTES.size)
        NOTES.forEachIndexed { n, freq ->
            for (i in 0 until noteLength) {
                val envelope = minOf(1.0, i / 600.0) * (1.0 - i.toDouble() / noteLength)
                pcm[n * noteLength + i] =
                    (sin(2 * PI * freq * i / SAMPLE_RATE) * envelope * 0.35 * Short.MAX_VALUE).toInt().toShort()
            }
        }
        return AudioTrack.Builder()
            .setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_MEDIA)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build(),
            )
            .setAudioFormat(
                AudioFormat.Builder()
                    .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                    .setSampleRate(SAMPLE_RATE)
                    .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                    .build(),
            )
            .setTransferMode(AudioTrack.MODE_STATIC)
            .setBufferSizeInBytes(pcm.size * 2)
            .build()
            .apply {
                write(pcm, 0, pcm.size)
                setLoopPoints(0, pcm.size, -1)
            }
    }

    private companion object {
        const val SAMPLE_RATE = 44_100
        const val VOLUME_FRACTION = 8f / 15f
        val NOTES = doubleArrayOf(523.25, 659.25, 783.99, 1046.5)
    }
}
