/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.viewmodels

import android.app.Application
import android.util.Log
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import org.lineageos.remotebuttons.audio.TestTone
import org.lineageos.remotebuttons.database.Brand
import org.lineageos.remotebuttons.ir.CodeSet
import org.lineageos.remotebuttons.ir.DeviceType
import org.lineageos.remotebuttons.remote.RemoteProgrammers
import org.lineageos.remotebuttons.repository.SetupRepository

class SetupViewModel(application: Application) : AndroidViewModel(application) {

    sealed interface Pairing {
        data object Idle : Pairing

        /** Sending code set [index] of [count] to the remote. */
        data class Programming(val index: Int, val count: Int) : Pairing

        /** The remote has code set [index]; waiting for the user to try the volume buttons. */
        data class Ready(val index: Int, val count: Int) : Pairing

        data class Error(val index: Int, val count: Int, val message: String) : Pairing

        /** Every code set of the brand was tried. */
        data object Exhausted : Pairing
    }

    val repository = SetupRepository(application)
    private val programmers = RemoteProgrammers(application)
    private val tone = TestTone(application)

    private val _pairing = MutableStateFlow<Pairing>(Pairing.Idle)
    val pairing = _pairing.asStateFlow()

    private var codeSets: List<CodeSet> = emptyList()
    private var brand: Brand? = null
    private var job: Job? = null

    suspend fun brands(type: DeviceType) = repository.database.brands(type)

    suspend fun popularBrands(type: DeviceType) = repository.database.popularBrands(type)

    fun startPairing(type: DeviceType, brand: Brand) {
        this.brand = brand
        job?.cancel()
        job = viewModelScope.launch {
            codeSets = repository.database.codeSets(type, brand)
            tryCodeSet(0)
        }
    }

    val codeSetCount get() = codeSets.size

    /** "No": the volume did not respond, move on to the next code set. */
    fun next() {
        val current = currentIndex() ?: return
        if (current + 1 >= codeSets.size) {
            tone.stop()
            _pairing.value = Pairing.Exhausted
        } else {
            tryCodeSet(current + 1)
        }
    }

    fun retry() = tryCodeSet(currentIndex() ?: 0)

    fun restart() = tryCodeSet(0)

    fun pick(index: Int) = tryCodeSet(index)

    /** "Yes": keep the code set currently in the remote. */
    fun confirm() {
        tone.stop()
        val index = currentIndex() ?: return
        repository.setConfigured("${brand?.name} (${codeSets[index].id.substringAfterLast('/')})")
        _pairing.value = Pairing.Idle
    }

    fun stopPairing() {
        job?.cancel()
        tone.stop()
        _pairing.value = Pairing.Idle
    }

    /** Clears the remote's IR codes. */
    suspend fun clear(): Result<Unit> = runCatching {
        programmers.withRemote(repository.remoteAddress) { remote, session ->
            remote.programmer.clear(session)
        }
        repository.setConfigured(null)
    }.onFailure { Log.e(TAG, "Clearing the remote failed", it) }

    private fun currentIndex() = when (val p = _pairing.value) {
        is Pairing.Programming -> p.index
        is Pairing.Ready -> p.index
        is Pairing.Error -> p.index
        else -> null
    }

    private fun tryCodeSet(index: Int) {
        job?.cancel()
        tone.stop()
        val count = codeSets.size
        _pairing.value = Pairing.Programming(index, count)
        job = viewModelScope.launch {
            _pairing.value = runCatching {
                programmers.withRemote(repository.remoteAddress) { remote, session ->
                    val codeSet = codeSets[index]
                    check(remote.programmer.supports(codeSet)) { "This remote cannot send these codes" }
                    remote.programmer.program(session, codeSet)
                    repository.remoteAddress = remote.address
                }
            }.fold(
                onSuccess = {
                    tone.start()
                    Pairing.Ready(index, count)
                },
                onFailure = {
                    Log.e(TAG, "Programming code set $index failed", it)
                    // The remote may have been paired again under a new address.
                    repository.remoteAddress = null
                    Pairing.Error(index, count, it.message ?: it.toString())
                },
            )
        }
    }

    override fun onCleared() {
        tone.stop()
    }

    private companion object {
        const val TAG = "SetupViewModel"
    }
}
