/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotevoice

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.os.Handler
import android.util.Log
import java.util.UUID

/**
 * The voice channel of a Xiaomi remote, carried by vendor reports of its HID service (as in
 * MiBluetooth's MitvVoice):
 * - output report 4 takes the stream enable command,
 * - input report 5 notifies the stream state,
 * - input reports 6 to 8 notify mSBC audio, one frame per 60 byte packet.
 *
 * MiBluetooth finds them by position, which depends on the other reports of the remote, so we look
 * them up by their report reference instead.
 *
 * The remote only powers its microphone between the enable and disable commands.
 */
@SuppressLint("MissingPermission")
class VoiceRemote(
    private val context: Context,
    val device: BluetoothDevice,
    private val handler: Handler,
    private val listener: Listener,
) : BluetoothGattCallback() {

    interface Listener {
        fun onReady(remote: VoiceRemote)

        /** The remote has no voice channel. */
        fun onUnsupported(remote: VoiceRemote)

        fun onClosed(remote: VoiceRemote)

        fun onAudio(remote: VoiceRemote, data: ByteArray)
    }

    private var gatt: BluetoothGatt? = null
    private var enableChar: BluetoothGattCharacteristic? = null
    private var stateChar: BluetoothGattCharacteristic? = null
    private val audioChars = mutableListOf<BluetoothGattCharacteristic>()
    private val references = mutableMapOf<BluetoothGattCharacteristic, Pair<Int, Int>>()

    /** GATT operations run one at a time; each returns false if it could not be started. */
    private val operations = ArrayDeque<() -> Boolean>()
    private var busy = false
    private var mtu = 23

    var ready = false
        private set

    /** Whether we asked the remote to stream. */
    var streaming = false
        private set

    fun open() {
        Log.d(TAG, "$device: connecting")
        gatt =
            device.connectGatt(
                context,
                false,
                this,
                BluetoothDevice.TRANSPORT_LE,
                BluetoothDevice.PHY_LE_1M_MASK,
                handler,
            )
    }

    fun close() {
        if (streaming) setStreaming(false)
        ready = false
        operations.clear()
        gatt?.let {
            it.disconnect()
            it.close()
        }
        gatt = null
    }

    fun setStreaming(on: Boolean) {
        val char = enableChar ?: return
        if (streaming == on) return
        streaming = on
        Log.i(TAG, "$device: ${if (on) "starting" else "stopping"} voice stream")
        val command = ByteArray(20)
        command[3] = 0x80.toByte()
        command[4] = if (on) 1 else 0
        enqueue { write(char, command) }
    }

    override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
        if (gatt != this.gatt) return
        if (newState == BluetoothProfile.STATE_CONNECTED) {
            if (!gatt.discoverServices()) fail("discoverServices refused")
        } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
            Log.d(TAG, "$device: disconnected ($status)")
            streaming = false
            close()
            listener.onClosed(this)
        }
    }

    override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
        if (gatt != this.gatt) return
        if (status != BluetoothGatt.GATT_SUCCESS) {
            fail("service discovery failed: $status")
            return
        }
        val reports =
            gatt.getService(HID_SERVICE)?.characteristics?.filter { it.uuid == HID_REPORT }
                ?: emptyList()
        if (reports.size < 8) {
            Log.d(TAG, "$device: no voice channel")
            listener.onUnsupported(this)
            return
        }
        references.clear()
        reports.forEach { char ->
            char.getDescriptor(REPORT_REFERENCE)?.let { enqueue { gatt.readDescriptor(it) } }
        }
        enqueue {
            resolve(gatt, reports)
            false
        }
    }

    /** Picks the voice reports once their references are known, then enables them. */
    private fun resolve(gatt: BluetoothGatt, reports: List<BluetoothGattCharacteristic>) {
        fun report(id: Int, type: Int) = reports.firstOrNull { references[it] == id to type }
        enableChar = report(ENABLE_REPORT_ID, REPORT_TYPE_OUTPUT)
        stateChar = report(STATE_REPORT_ID, REPORT_TYPE_INPUT)
        audioChars.clear()
        AUDIO_REPORT_IDS.forEach { id -> report(id, REPORT_TYPE_INPUT)?.let { audioChars.add(it) } }
        val stateChar = stateChar
        if (enableChar == null || stateChar == null || audioChars.size != AUDIO_REPORT_IDS.size) {
            Log.d(TAG, "$device: no voice channel")
            listener.onUnsupported(this)
            return
        }

        enqueue {
            // 60 byte audio notifications need a larger MTU than the default
            gatt.requestMtu(MTU)
        }
        (listOf(stateChar) + audioChars).forEach { char ->
            enqueue {
                gatt.setCharacteristicNotification(char, true)
                val cccd = char.getDescriptor(CCCD) ?: return@enqueue false
                gatt.writeDescriptor(cccd, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE) ==
                    BluetoothGatt.GATT_SUCCESS
            }
        }
        enqueue {
            ready = true
            Log.i(TAG, "$device: voice channel ready (MTU ${this.mtu})")
            listener.onReady(this)
            false
        }
    }

    override fun onMtuChanged(gatt: BluetoothGatt, mtu: Int, status: Int) {
        if (status == BluetoothGatt.GATT_SUCCESS) this.mtu = mtu
        done()
    }

    override fun onDescriptorRead(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        status: Int,
        value: ByteArray,
    ) {
        // Report ID, then report type
        if (status == BluetoothGatt.GATT_SUCCESS && value.size >= 2) {
            references[descriptor.characteristic] = value[0].toInt() to value[1].toInt()
        }
        done()
    }

    override fun onDescriptorWrite(
        gatt: BluetoothGatt,
        descriptor: BluetoothGattDescriptor,
        status: Int,
    ) {
        if (status != BluetoothGatt.GATT_SUCCESS) {
            Log.w(TAG, "$device: enabling notifications failed: $status")
        }
        done()
    }

    override fun onCharacteristicWrite(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        status: Int,
    ) {
        if (status != BluetoothGatt.GATT_SUCCESS) {
            Log.w(TAG, "$device: stream command failed: $status")
        }
        done()
    }

    override fun onCharacteristicChanged(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic,
        value: ByteArray,
    ) {
        if (characteristic in audioChars) {
            listener.onAudio(this, value)
        } else if (characteristic == stateChar) {
            val started = value.isNotEmpty() && value[0].toInt() == 1
            Log.d(TAG, "$device: remote reports stream ${if (started) "started" else "stopped"}")
        }
    }

    private fun write(char: BluetoothGattCharacteristic, value: ByteArray): Boolean {
        val gatt = gatt ?: return false
        // The report's default write type, as MiBluetooth does
        return gatt.writeCharacteristic(char, value, char.writeType) == BluetoothGatt.GATT_SUCCESS
    }

    private fun enqueue(operation: () -> Boolean) {
        operations.addLast(operation)
        runNext()
    }

    private fun done() {
        busy = false
        runNext()
    }

    private fun runNext() {
        while (!busy && operations.isNotEmpty()) {
            busy = operations.removeFirst()()
        }
    }

    private fun fail(reason: String) {
        Log.w(TAG, "$device: $reason")
        close()
        listener.onClosed(this)
    }

    companion object {
        private const val TAG = "VoiceRemote"
        private const val MTU = 120

        private const val REPORT_TYPE_INPUT = 1
        private const val REPORT_TYPE_OUTPUT = 2
        private const val ENABLE_REPORT_ID = 4
        private const val STATE_REPORT_ID = 5
        private val AUDIO_REPORT_IDS = listOf(6, 7, 8)

        private val HID_SERVICE = UUID.fromString("00001812-0000-1000-8000-00805f9b34fb")
        private val HID_REPORT = UUID.fromString("00002a4d-0000-1000-8000-00805f9b34fb")
        private val REPORT_REFERENCE = UUID.fromString("00002908-0000-1000-8000-00805f9b34fb")
        private val CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")
    }
}
