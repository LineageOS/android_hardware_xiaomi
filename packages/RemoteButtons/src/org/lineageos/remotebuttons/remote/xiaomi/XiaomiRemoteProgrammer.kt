/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.remote.xiaomi

import android.bluetooth.BluetoothGattCharacteristic
import java.util.UUID
import org.lineageos.remotebuttons.ir.CodeSet
import org.lineageos.remotebuttons.ir.IrCode
import org.lineageos.remotebuttons.ir.IrKey
import org.lineageos.remotebuttons.remote.GattSession
import org.lineageos.remotebuttons.remote.RemoteProgrammer

/**
 * Xiaomi BLE remotes (Xiaomi TV Box S 2nd Gen): IR codes are written to a vendor GATT service, as
 * the stock MiBluetooth app does (`com.mi.bluetooth...ircode.IrCodeDispatcher`).
 *
 * The descriptor block goes to [PARAM_CHAR], then the power, volume up and volume down blocks to
 * [KEY_CHAR]. A block is `[type u16 LE][length u16 LE][data]`; one that fits a single write (`size
 * <= mtu - 3`) is written plainly, a longer one as a reliable write in `mtu - 5` chunks.
 */
class XiaomiRemoteProgrammer : RemoteProgrammer {

    class Factory : RemoteProgrammer.Factory {
        override val serviceUuid: UUID = SERVICE

        override fun create() = XiaomiRemoteProgrammer()
    }

    override fun supports(codeSet: CodeSet) =
        codeSet.rawParameters != null &&
            codeSet.keys.values.all { it is IrCode.Raw && it.format == RAW_FORMAT }

    override suspend fun program(session: GattSession, codeSet: CodeSet) {
        val descriptor = requireNotNull(codeSet.rawParameters) { "Code set without descriptor" }
        val keys =
            KEY_ORDER.map { (key, type) ->
                val data = (codeSet.keys[key] as? IrCode.Raw)?.data.orEmpty()
                block(type, data)
            }
        write(session, block(TYPE_PARAM, descriptor), keys)
    }

    override suspend fun clear(session: GattSession) =
        write(session, block(TYPE_PARAM, ""), KEY_ORDER.map { (_, type) -> block(type, "") })

    private suspend fun write(session: GattSession, param: ByteArray, keys: List<ByteArray>) {
        val service = checkNotNull(session.service(SERVICE)) { "Remote has no IR service" }
        val mtu = session.requestMtu(MTU)
        writeBlock(session, service.getCharacteristic(PARAM_CHAR), param, mtu)
        keys.forEach { writeBlock(session, service.getCharacteristic(KEY_CHAR), it, mtu) }
    }

    private suspend fun writeBlock(
        session: GattSession,
        characteristic: BluetoothGattCharacteristic,
        data: ByteArray,
        mtu: Int,
    ) {
        if (data.size <= mtu - 3) {
            session.write(characteristic, data)
        } else {
            session.reliableWrite(
                characteristic,
                data.asList().chunked(mtu - 5).map { it.toByteArray() },
            )
        }
    }

    companion object {
        /** [IrCode.Raw.format] of codes from the stock Xiaomi database. */
        const val RAW_FORMAT = "xiaomi"

        val SERVICE: UUID = UUID.fromString("0000a00a-0000-1000-8000-00805f9b34fb")
        val KEY_CHAR: UUID = UUID.fromString("0000b005-0000-1000-8000-00805f9b34fb")
        val PARAM_CHAR: UUID = UUID.fromString("0000c001-0000-1000-8000-00805f9b34fb")

        private const val MTU = 160
        private const val TYPE_PARAM = 0
        private val KEY_ORDER =
            listOf(IrKey.POWER to 1, IrKey.VOLUME_UP to 2, IrKey.VOLUME_DOWN to 3)

        /**
         * Builds one block. Stock MiBluetooth encodes a toggling value "A&B" as `[type][len
         * A][A][len A][first len(B) bytes of A]` (B is discarded, as verified in its bytecode);
         * that is what the remote has always received, so it is reproduced, except where stock
         * would fail (B longer than A).
         */
        fun block(type: Int, hex: String): ByteArray {
            val split = hex.indexOf('&')
            if (split < 0) {
                val data = hexToBytes(hex)
                return u16le(type) + u16le(data.size) + data
            }
            val a = hexToBytes(hex.substring(0, split))
            val b = hexToBytes(hex.substring(split + 1))
            return if (b.size <= a.size) {
                u16le(type) + u16le(a.size) + a + u16le(a.size) + a.copyOf(b.size)
            } else {
                u16le(type) + u16le(a.size) + a + u16le(b.size) + b
            }
        }

        private fun u16le(v: Int) = byteArrayOf((v and 0xFF).toByte(), (v shr 8 and 0xFF).toByte())

        private fun hexToBytes(hex: String) =
            ByteArray(hex.length / 2) { i -> hex.substring(i * 2, i * 2 + 2).toInt(16).toByte() }
    }
}
