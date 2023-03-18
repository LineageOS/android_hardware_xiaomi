/*
 * SPDX-FileCopyrightText: 2015-2016 The CyanogenMod Project
 * SPDX-FileCopyrightText: 2017,2019-2023 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.settings.doze

import android.app.Activity
import android.app.AlertDialog
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.widget.Switch

import androidx.preference.Preference
import androidx.preference.Preference.OnPreferenceChangeListener
import androidx.preference.PreferenceCategory
import androidx.preference.PreferenceFragmentCompat
import androidx.preference.SwitchPreference

import com.android.settingslib.widget.MainSwitchPreference
import com.android.settingslib.widget.OnMainSwitchChangeListener

import org.lineageos.settings.doze.DozeUtils.isDozeEnabled

class DozeSettingsFragment : PreferenceFragmentCompat(), OnPreferenceChangeListener,
    OnMainSwitchChangeListener {
    private lateinit var switchBar: MainSwitchPreference
    private lateinit var alwaysOnDisplayPreference: SwitchPreference

    private var pickUpPreference: SwitchPreference? = null
    private var handwavePreference: SwitchPreference? = null
    private var pocketPreference: SwitchPreference? = null

    private val handler = Handler(Looper.getMainLooper())

    override fun onCreatePreferences(savedInstanceState: Bundle?, rootKey: String?) {
        addPreferencesFromResource(R.xml.doze_settings)

        val activity = requireActivity()

        val prefs = activity.getSharedPreferences("doze_settings", Activity.MODE_PRIVATE)

        if (savedInstanceState == null && !prefs.getBoolean("first_help_shown", false)) {
            showHelp()
        }

        val dozeEnabled = isDozeEnabled(activity)

        switchBar = findPreference(DozeUtils.DOZE_ENABLE)!!
        switchBar.addOnSwitchChangeListener(this)
        switchBar.setChecked(dozeEnabled)

        alwaysOnDisplayPreference = findPreference(DozeUtils.ALWAYS_ON_DISPLAY)!!
        alwaysOnDisplayPreference.isEnabled = dozeEnabled
        alwaysOnDisplayPreference.isChecked = DozeUtils.isAlwaysOnEnabled(activity)
        alwaysOnDisplayPreference.onPreferenceChangeListener = this

        val pickupSensorCategory =
            preferenceScreen.findPreference<PreferenceCategory>(DozeUtils.CATEG_PICKUP_SENSOR)
        val proximitySensorCategory =
            preferenceScreen.findPreference<PreferenceCategory>(DozeUtils.CATEG_PROX_SENSOR)

        pickUpPreference = findPreference(DozeUtils.GESTURE_PICK_UP_KEY)
        pickUpPreference?.isEnabled = dozeEnabled
        pickUpPreference?.onPreferenceChangeListener = this

        handwavePreference = findPreference(DozeUtils.GESTURE_HAND_WAVE_KEY)
        handwavePreference?.isEnabled = dozeEnabled
        handwavePreference?.onPreferenceChangeListener = this

        pocketPreference = findPreference(DozeUtils.GESTURE_POCKET_KEY)
        pocketPreference?.isEnabled = dozeEnabled
        pocketPreference?.onPreferenceChangeListener = this

        // Hide proximity sensor related features if the device doesn't support them
        if (!DozeUtils.getProxCheckBeforePulse(activity)) {
            preferenceScreen.removePreference(proximitySensorCategory)
        }

        // Hide AOD if not supported and set all its dependents otherwise
        if (!DozeUtils.alwaysOnDisplayAvailable(activity)) {
            preferenceScreen.removePreference(alwaysOnDisplayPreference)
        } else {
            pickupSensorCategory?.dependency = DozeUtils.ALWAYS_ON_DISPLAY
            proximitySensorCategory?.dependency = DozeUtils.ALWAYS_ON_DISPLAY
        }
    }

    override fun onPreferenceChange(preference: Preference, newValue: Any): Boolean {
        val activity = requireActivity()

        if (DozeUtils.ALWAYS_ON_DISPLAY == preference.key) {
            DozeUtils.enableAlwaysOn(activity, (newValue as Boolean))
        }

        handler.post { DozeService.checkService(activity) }

        return true
    }

    override fun onSwitchChanged(switchView: Switch?, isChecked: Boolean) {
        val activity = requireActivity()

        DozeUtils.enableDoze(activity, isChecked)
        DozeService.checkService(activity)

        switchBar.setChecked(isChecked)

        if (!isChecked) {
            DozeUtils.enableAlwaysOn(activity, false)
            alwaysOnDisplayPreference.isChecked = false
        }

        alwaysOnDisplayPreference.isEnabled = isChecked
        pickUpPreference?.isEnabled = isChecked
        handwavePreference?.isEnabled = isChecked
        pocketPreference?.isEnabled = isChecked
    }

    private fun showHelp() {
        val activity = requireActivity()

        AlertDialog.Builder(context)
            .setTitle(R.string.doze_settings_help_title)
            .setMessage(R.string.doze_settings_help_text)
            .setNegativeButton(R.string.dialog_ok) { _, _ ->
                activity.getSharedPreferences("doze_settings", Activity.MODE_PRIVATE)
                    .edit()
                    .putBoolean("first_help_shown", true)
                    .apply()
            }
            .show()
    }
}
