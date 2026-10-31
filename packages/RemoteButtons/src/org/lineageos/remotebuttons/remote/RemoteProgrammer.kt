/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.remote

import java.util.UUID
import org.lineageos.remotebuttons.ir.CodeSet

/**
 * Programs the IR buttons of one family of Bluetooth remotes.
 *
 * This is the only part that depends on the remote. To support another remote:
 * 1. implement this interface and a [Factory],
 * 2. add the factory to [RemoteProgrammers].
 */
interface RemoteProgrammer {
    /** Whether every code of [codeSet] can be sent to this remote. */
    fun supports(codeSet: CodeSet): Boolean

    /** Makes the remote send [codeSet]'s codes from its power and volume buttons. */
    suspend fun program(session: GattSession, codeSet: CodeSet)

    /** Makes the remote stop sending IR. */
    suspend fun clear(session: GattSession)

    interface Factory {
        /** The GATT service a remote of this family exposes for IR programming. */
        val serviceUuid: UUID

        fun create(): RemoteProgrammer
    }
}
