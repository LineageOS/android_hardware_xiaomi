/*
 * SPDX-FileCopyrightText: 2019 The Android Open Source Project
 * SPDX-FileCopyrightText: 2025 Paranoid Android
 * SPDX-License-Identifier: Apache-2.0
 */

package com.xiaomi.mtb

import android.content.ContentProvider
import android.content.ContentValues
import android.database.Cursor
import android.net.Uri
import android.os.Bundle
import com.android.settingslib.drawer.TileUtils.META_DATA_PREFERENCE_SUMMARY
import com.xiaomi.mtb.EsimController

class SummaryProvider : ContentProvider() {

    private val esimController by lazy { EsimController.getInstance(requireContext()) }

    override fun call(method: String, uri: String?, extras: Bundle?): Bundle {
        val bundle = Bundle()
        val summary =
            when (method) {
                KEY_ESIM -> getESimSummary()
                else -> throw IllegalArgumentException("Unknown method: $method")
            }
        bundle.putString(META_DATA_PREFERENCE_SUMMARY, summary)
        return bundle
    }

    override fun onCreate(): Boolean = true

    override fun query(
        uri: Uri,
        projection: Array<out String>?,
        selection: String?,
        selectionArgs: Array<out String>?,
        sortOrder: String?,
    ): Cursor? = throw UnsupportedOperationException()

    override fun getType(uri: Uri): String? = throw UnsupportedOperationException()

    override fun insert(uri: Uri, values: ContentValues?): Uri? =
        throw UnsupportedOperationException()

    override fun delete(uri: Uri, selection: String?, selectionArgs: Array<out String>?): Int =
        throw UnsupportedOperationException()

    override fun update(
        uri: Uri,
        values: ContentValues?,
        selection: String?,
        selectionArgs: Array<out String>?,
    ): Int = throw UnsupportedOperationException()

    private fun getESimSummary(): String {
        val context = context ?: return ""
        return when {
            esimController.getEsimEnabled() -> context.getString(R.string.esim_summary_enabled)
            esimController.getEsimSupported() -> context.getString(R.string.esim_summary_disabled)
            else -> context.getString(R.string.esim_summary_unsupported)
        }
    }

    companion object {
        private const val KEY_ESIM = "esim"
    }
}
