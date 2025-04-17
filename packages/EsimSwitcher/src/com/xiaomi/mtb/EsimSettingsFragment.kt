/*
 * SPDX-FileCopyrightText: 2025 Paranoid Android
 * SPDX-License-Identifier: Apache-2.0
 */

package com.xiaomi.mtb

import android.os.Bundle
import android.util.Log
import android.widget.CompoundButton
import androidx.appcompat.app.AlertDialog
import com.android.settingslib.widget.FooterPreference
import com.android.settingslib.widget.MainSwitchPreference
import com.android.settingslib.widget.SettingsBasePreferenceFragment

class EsimSettingsFragment :
    SettingsBasePreferenceFragment(), CompoundButton.OnCheckedChangeListener {

    companion object {
        private const val TAG = "EsimSettingsFragment"
        private val DEBUG = Log.isLoggable(TAG, Log.DEBUG)
    }

    private val esimController by lazy { EsimController.getInstance(requireContext()) }

    private val switchBar by lazy { findPreference<MainSwitchPreference>("esim_enable")!! }
    private val footerPref by lazy { findPreference<FooterPreference>("esim_footer")!! }

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        if (DEBUG) Log.d(TAG, "onCreatePreferences")
        setPreferencesFromResource(R.xml.settings_esim, rootKey)

        if (esimController.getEsimSupported()) {
            switchBar.addOnSwitchChangeListener(this)
            switchBar.isChecked = esimController.getEsimEnabled()
            switchBar.isEnabled = true
            footerPref.title = getString(R.string.esim_footer_note)
        }
    }

    override fun onCheckedChanged(buttonView: CompoundButton, isChecked: Boolean) {
        if (DEBUG) Log.d(TAG, "onCheckedChanged: $isChecked")
        if (esimController.getEsimActive()) {
            if (isChecked) return
            switchBar.isChecked = true
            AlertDialog.Builder(requireContext())
                .setTitle(R.string.esim_warning_title)
                .setMessage(R.string.esim_warning_message)
                .setPositiveButton(android.R.string.ok, null)
                .setCancelable(false)
                .show()
        } else {
            esimController.setEsimEnabled(isChecked)
        }
    }
}
