/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotevoice

import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothUuid
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.os.Handler
import android.os.HandlerThread
import android.util.Log

/**
 * Offers the microphone of connected remotes while an app captures audio.
 *
 * The remote's microphone draws power, so it is only streaming while an app records, at most for
 * [MAX_STREAM_MS] per capture and never with the screen off.
 */
@SuppressLint("MissingPermission")
class RemoteVoiceManager(private val context: Context) : VoiceRemote.Listener {

    private val handler = Handler(HandlerThread(TAG).apply { start() }.looper)
    private val adapter = context.getSystemService(BluetoothManager::class.java)!!.adapter

    private val remotes = mutableMapOf<String, VoiceRemote>()
    /** Remotes without a voice channel, so we don't probe them on every reconnection. */
    private val unsupported = mutableSetOf<String>()

    private var injector: MicInjector? = null
    private var capturing = false

    private val decoder = MsbcDecoder()
    private val samples = ShortArray(MsbcDecoder.SAMPLES_PER_FRAME)
    private var lastRemote: VoiceRemote? = null

    private val stopStreaming = Runnable {
        Log.i(TAG, "capture still running after ${MAX_STREAM_MS / 1000} s, releasing the remote")
        releaseRemotes()
    }

    private val receiver =
        object : BroadcastReceiver() {
            override fun onReceive(context: Context, intent: Intent) {
                when (intent.action) {
                    BluetoothDevice.ACTION_ACL_CONNECTED -> intent.device()?.let { probe(it) }

                    BluetoothDevice.ACTION_ACL_DISCONNECTED -> intent.device()?.let { drop(it) }

                    BluetoothDevice.ACTION_BOND_STATE_CHANGED ->
                        intent.device()?.let {
                            when (it.bondState) {
                                BluetoothDevice.BOND_BONDED -> if (it.isConnected) probe(it)
                                BluetoothDevice.BOND_NONE -> {
                                    unsupported.remove(it.address)
                                    drop(it)
                                }
                            }
                        }

                    BluetoothAdapter.ACTION_STATE_CHANGED ->
                        if (adapter.state == BluetoothAdapter.STATE_ON) probeConnected()

                    Intent.ACTION_SCREEN_OFF -> {
                        handler.removeCallbacks(stopStreaming)
                        releaseRemotes()
                    }
                }
            }
        }

    fun start() {
        val filter =
            IntentFilter().apply {
                addAction(BluetoothDevice.ACTION_ACL_CONNECTED)
                addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED)
                addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED)
                addAction(BluetoothAdapter.ACTION_STATE_CHANGED)
                addAction(Intent.ACTION_SCREEN_OFF)
            }
        context.registerReceiver(receiver, filter, null, handler, Context.RECEIVER_NOT_EXPORTED)
        handler.post { probeConnected() }
    }

    private fun probeConnected() {
        adapter.bondedDevices?.filter { it.isConnected }?.forEach { probe(it) }
    }

    /** Looks for a voice channel on a paired LE HID device. */
    private fun probe(device: BluetoothDevice) {
        if (device.address in remotes || device.address in unsupported) return
        if (device.bondState != BluetoothDevice.BOND_BONDED) return
        if (device.type == BluetoothDevice.DEVICE_TYPE_CLASSIC) return
        val uuids = device.uuids
        if (uuids != null && BluetoothUuid.HOGP !in uuids) return

        val remote = VoiceRemote(context, device, handler, this)
        remotes[device.address] = remote
        remote.open()
    }

    private fun drop(device: BluetoothDevice) {
        remotes[device.address]?.let {
            it.close()
            onClosed(it)
        }
    }

    override fun onReady(remote: VoiceRemote) {
        if (injector == null) {
            injector =
                try {
                    MicInjector(context, handler, ::onCaptureChanged).takeIf { it.register() }
                } catch (e: RuntimeException) {
                    Log.e(TAG, "Failed to offer the remote microphone", e)
                    null
                }
        }
        // A remote that (re)connects during a capture joins it
        if (capturing) remote.setStreaming(true)
    }

    override fun onUnsupported(remote: VoiceRemote) {
        unsupported.add(remote.device.address)
        remote.close()
        onClosed(remote)
    }

    override fun onClosed(remote: VoiceRemote) {
        if (remotes[remote.device.address] === remote) remotes.remove(remote.device.address)
        if (remotes.values.none { it.ready }) {
            injector?.unregister()
            injector = null
            capturing = false
            handler.removeCallbacks(stopStreaming)
        }
    }

    override fun onAudio(remote: VoiceRemote, data: ByteArray) {
        if (!remote.streaming) return
        if (remote !== lastRemote) {
            decoder.reset()
            lastRemote = remote
        }
        // Each packet is a 0x01 marker, a sequence number and an mSBC frame, padded to 60 bytes
        var offset = 0
        while (offset + 2 + MsbcDecoder.FRAME_SIZE <= data.size) {
            if (data[offset].toInt() == 1 && decoder.decode(data, offset + 2, samples, 0)) {
                injector?.write(samples, samples.size)
            }
            offset += PACKET_SIZE
        }
    }

    private fun onCaptureChanged(active: Boolean) {
        capturing = active
        handler.removeCallbacks(stopStreaming)
        if (active) {
            lastRemote = null
            handler.postDelayed(stopStreaming, MAX_STREAM_MS)
        }
        remotes.values.filter { it.ready }.forEach { it.setStreaming(active) }
    }

    /** Stops streaming until the next capture, also for remotes that connect meanwhile. */
    private fun releaseRemotes() {
        capturing = false
        remotes.values.forEach { it.setStreaming(false) }
    }

    private fun Intent.device(): BluetoothDevice? =
        getParcelableExtra(BluetoothDevice.EXTRA_DEVICE, BluetoothDevice::class.java)

    companion object {
        private const val TAG = "RemoteVoice"
        private const val PACKET_SIZE = 60
        private const val MAX_STREAM_MS = 60_000L
    }
}
