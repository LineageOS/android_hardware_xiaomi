/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.remote

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattService
import android.bluetooth.BluetoothProfile
import android.content.Context
import android.util.Log
import kotlinx.coroutines.channels.Channel
import kotlinx.coroutines.withTimeout
import java.io.Closeable
import java.util.UUID

/**
 * A GATT client connection to a remote, with suspending one-at-a-time operations.
 *
 * Opening our own client does not disturb the system's HID connection to the remote: both share
 * the same link. Callers need the BLUETOOTH_CONNECT permission.
 */
@SuppressLint("MissingPermission")
class GattSession private constructor(private val gatt: BluetoothGatt, private val events: Channel<Event>) :
    Closeable {

    private sealed interface Event {
        data class Connection(val status: Int, val state: Int) : Event
        data class Services(val status: Int) : Event
        data class Mtu(val mtu: Int, val status: Int) : Event
        data class Write(val status: Int) : Event
        data class ReliableDone(val status: Int) : Event
    }

    /** The negotiated ATT MTU; 23 until [requestMtu] succeeds. */
    var mtu = DEFAULT_MTU
        private set

    fun service(uuid: UUID): BluetoothGattService? = gatt.getService(uuid)

    fun hasService(uuid: UUID) = service(uuid) != null

    /** All services the remote exposes. */
    val services: List<BluetoothGattService> get() = gatt.services

    suspend fun requestMtu(mtu: Int): Int {
        if (gatt.requestMtu(mtu)) {
            val result = await<Event.Mtu>()
            if (result.status == BluetoothGatt.GATT_SUCCESS) this.mtu = result.mtu
        }
        return this.mtu
    }

    /**
     * Writes [value], with a response when the characteristic supports it and without one
     * otherwise (as the platform does by default), waiting until the write is done.
     */
    suspend fun write(characteristic: BluetoothGattCharacteristic, value: ByteArray) {
        val writeType = if (characteristic.properties and BluetoothGattCharacteristic.PROPERTY_WRITE != 0) {
            BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        } else {
            BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
        }
        val started = gatt.writeCharacteristic(characteristic, value, writeType)
        check(started == BluetoothGatt.GATT_SUCCESS) { "writeCharacteristic refused: $started" }
        val result = await<Event.Write>()
        check(result.status == BluetoothGatt.GATT_SUCCESS) { "write failed: ${result.status}" }
    }

    /** Writes [chunks] as one reliable-write transaction. */
    suspend fun reliableWrite(characteristic: BluetoothGattCharacteristic, chunks: List<ByteArray>) {
        check(gatt.beginReliableWrite()) { "beginReliableWrite refused" }
        try {
            chunks.forEach { write(characteristic, it) }
            check(gatt.executeReliableWrite()) { "executeReliableWrite refused" }
            val done = await<Event.ReliableDone>()
            check(done.status == BluetoothGatt.GATT_SUCCESS) { "reliable write failed: ${done.status}" }
        } catch (e: Exception) {
            gatt.abortReliableWrite()
            throw e
        }
    }

    override fun close() {
        gatt.disconnect()
        gatt.close()
    }

    private suspend inline fun <reified E : Event> await(): E = Companion.await(events)

    companion object {
        private const val TAG = "GattSession"
        private const val DEFAULT_MTU = 23
        private const val TIMEOUT_MS = 10_000L

        /** Connects to [device] and discovers its services. */
        @Suppress("DEPRECATION") // The replacement connectGatt overload needs API 37.
        suspend fun open(context: Context, device: BluetoothDevice): GattSession {
            val events = Channel<Event>(Channel.UNLIMITED)
            val callback = object : BluetoothGattCallback() {
                override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
                    events.trySend(Event.Connection(status, newState))
                }

                override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
                    events.trySend(Event.Services(status))
                }

                override fun onMtuChanged(gatt: BluetoothGatt, mtu: Int, status: Int) {
                    events.trySend(Event.Mtu(mtu, status))
                }

                override fun onCharacteristicWrite(
                    gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, status: Int,
                ) {
                    events.trySend(Event.Write(status))
                }

                override fun onReliableWriteCompleted(gatt: BluetoothGatt, status: Int) {
                    events.trySend(Event.ReliableDone(status))
                }
            }
            val gatt = device.connectGatt(context, false, callback, BluetoothDevice.TRANSPORT_LE)
            val session = GattSession(gatt, events)
            try {
                val connected = await<Event.Connection>(events)
                check(connected.state == BluetoothProfile.STATE_CONNECTED) { "connect failed: ${connected.status}" }
                check(gatt.discoverServices()) { "discoverServices refused" }
                val services = await<Event.Services>(events)
                check(services.status == BluetoothGatt.GATT_SUCCESS) { "service discovery failed: ${services.status}" }
                return session
            } catch (e: Exception) {
                session.close()
                throw e
            }
        }

        private suspend inline fun <reified E : Event> await(events: Channel<Event>): E = withTimeout(TIMEOUT_MS) {
            var result: E? = null
            while (result == null) {
                when (val event = events.receive()) {
                    is E -> result = event
                    is Event.Connection -> check(event.state == BluetoothProfile.STATE_CONNECTED) {
                        "remote disconnected (status ${event.status})"
                    }

                    else -> Log.d(TAG, "Ignoring $event while waiting for ${E::class.simpleName}")
                }
            }
            result
        }
    }
}
