/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.settings

import android.Manifest
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.StrictMode
import androidx.slice.Slice
import androidx.slice.SliceProvider
import com.android.tv.twopanelsettings.slices.builders.PreferenceSliceBuilder
import com.android.tv.twopanelsettings.slices.builders.PreferenceSliceBuilder.RowBuilder
import org.lineageos.remotebuttons.MainActivity
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.repository.SetupRepository

/**
 * The "Set up remote buttons" page of TV Settings > Remotes & Accessories.
 *
 * TV Settings shows the entry when its `bluetooth_ir_entry_slice_uri` string points at this
 * provider (set with a resource overlay in the device tree) and renders these slices in its panels.
 * TV Settings holds WRITE_SECURE_SETTINGS, so it is granted access without a prompt.
 */
class IrSliceProvider : SliceProvider(Manifest.permission.WRITE_SECURE_SETTINGS) {

    override fun onCreateSliceProvider() = true

    override fun onBindSlice(sliceUri: Uri): Slice? {
        val context = context ?: return null
        // Slices are bound under a StrictMode policy that forbids disk access, but the page
        // shows saved state; like AOSP Settings' slice provider, lift it while building.
        val oldPolicy = StrictMode.getThreadPolicy()
        StrictMode.setThreadPolicy(StrictMode.ThreadPolicy.LAX)
        return try {
            val repository = SetupRepository(context)
            when (sliceUri.path) {
                PATH_MAIN -> mainSlice(context, sliceUri, repository)
                else -> null
            }
        } finally {
            StrictMode.setThreadPolicy(oldPolicy)
        }
    }

    private fun mainSlice(context: Context, sliceUri: Uri, repository: SetupRepository): Slice {
        return PreferenceSliceBuilder(context, sliceUri)
            .addScreenTitle(RowBuilder().setTitle(context.getString(R.string.settings_title)))
            .addPreference(
                RowBuilder()
                    .setKey(KEY_SETUP)
                    .setTitle(context.getString(R.string.action_set_up))
                    .setSubtitle(
                        repository.configured?.let {
                            context.getString(R.string.status_configured, it)
                        } ?: context.getString(R.string.status_not_configured)
                    )
                    .setPendingIntent(launch(context, MainActivity.START_SETUP))
            )
            // Always offered: the remote may hold codes set up elsewhere, e.g. by the stock
            // firmware.
            .addPreference(
                RowBuilder()
                    .setKey(KEY_CLEAR)
                    .setTitle(context.getString(R.string.action_clear))
                    .setPendingIntent(launch(context, MainActivity.START_CLEAR))
            )
            .build()
    }

    private fun launch(context: Context, start: String): PendingIntent =
        PendingIntent.getActivity(
            context,
            start.hashCode(),
            Intent(context, MainActivity::class.java)
                .putExtra(MainActivity.EXTRA_START, start)
                .addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TASK),
            PendingIntent.FLAG_IMMUTABLE or PendingIntent.FLAG_UPDATE_CURRENT,
        )

    companion object {
        private const val PATH_MAIN = "/ir"
        private const val KEY_SETUP = "setup"
        private const val KEY_CLEAR = "clear"

        private fun uri(context: Context, path: String): Uri =
            Uri.parse("content://${context.getString(R.string.slice_authority)}$path")

        /** Asks TV Settings to redraw the pages. */
        fun notifyChanged(context: Context) {
            context.contentResolver.notifyChange(uri(context, PATH_MAIN), null)
        }
    }
}
