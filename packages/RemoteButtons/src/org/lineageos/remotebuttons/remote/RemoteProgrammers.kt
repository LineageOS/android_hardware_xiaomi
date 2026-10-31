/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.remote

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.util.Log
import org.lineageos.remotebuttons.R

/** Finds the paired remote and the [RemoteProgrammer] that can program it. */
@SuppressLint("MissingPermission")
class RemoteProgrammers(private val context: Context) {

    class NoRemoteException : Exception("No paired remote supports IR programming")

    data class Remote(val address: String, val programmer: RemoteProgrammer)

    /** Programmer factories enabled for this device, from `config_remoteProgrammers`. */
    val factories: List<RemoteProgrammer.Factory> by lazy {
        context.resources.getStringArray(R.array.config_remoteProgrammers).mapNotNull { name ->
            runCatching { Class.forName(name).getDeclaredConstructor().newInstance() as RemoteProgrammer.Factory }
                .onFailure { Log.e(TAG, "Cannot load remote programmer $name", it) }
                .getOrNull()
        }
    }

    private val adapter get() = context.getSystemService(BluetoothManager::class.java).adapter

    /**
     * Opens a session with the remote at [address], or with the first paired remote that exposes
     * a supported service when [address] is null, and runs [block] with it.
     */
    suspend fun <T> withRemote(address: String?, block: suspend (Remote, GattSession) -> T): T {
        val candidates = adapter.bondedDevices
            .filter { it.type == BluetoothDevice.DEVICE_TYPE_LE || it.type == BluetoothDevice.DEVICE_TYPE_DUAL }
            .filter { address == null || it.address == address }
        for (device in candidates) {
            val session = runCatching { GattSession.open(context, device) }
                .onFailure { Log.w(TAG, "Cannot connect to ${device.address}", it) }
                .getOrNull() ?: continue
            session.use {
                val factory = factories.firstOrNull { session.hasService(it.serviceUuid) } ?: return@use
                return block(Remote(device.address, factory.create()), session)
            }
        }
        throw NoRemoteException()
    }

    /**
     * Read-only diagnostics: lists the services of every paired LE device and which programmer
     * matches. Nothing is written to the remotes.
     */
    suspend fun describe(): String = buildString {
        for (device in adapter.bondedDevices.filter { it.type != BluetoothDevice.DEVICE_TYPE_CLASSIC }) {
            appendLine("${device.name} ${device.address}")
            val session = runCatching { GattSession.open(context, device) }
                .onFailure { appendLine("  cannot connect: ${it.message}") }
                .getOrNull() ?: continue
            session.use {
                session.services.forEach { service ->
                    appendLine("  service ${service.uuid}")
                    service.characteristics.forEach { appendLine("    characteristic ${it.uuid} properties=0x%x".format(it.properties)) }
                }
                val matches = factories.filter { session.hasService(it.serviceUuid) }
                appendLine("  programmers: " + matches.joinToString { it.javaClass.name }.ifEmpty { "none" })
            }
        }
    }.also { report -> report.lines().forEach { Log.i(TAG, it) } }

    private companion object {
        const val TAG = "RemoteProgrammers"
    }
}
