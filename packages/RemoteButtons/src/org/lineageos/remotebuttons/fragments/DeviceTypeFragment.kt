/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.ir.DeviceType

/** Asks what the remote should control. */
class DeviceTypeFragment : RemoteButtonsGuidedStepFragment() {

    override fun onCreateGuidance(savedInstanceState: Bundle?) = GuidanceStylist.Guidance(
        getString(R.string.device_type_title),
        null,
        getString(R.string.app_name),
        null,
    )

    override fun onCreateActions(actions: MutableList<GuidedAction>, savedInstanceState: Bundle?) {
        listOf(
            DeviceType.TV to R.string.device_type_tv,
            DeviceType.SOUNDBAR to R.string.device_type_soundbar,
            DeviceType.RECEIVER to R.string.device_type_receiver,
        ).forEach { (type, title) ->
            actions += GuidedAction.Builder(requireContext()).id(type.ordinal.toLong()).title(title).build()
        }
    }

    override fun onGuidedActionClicked(action: GuidedAction) {
        add(parentFragmentManager, BrandFragment.newInstance(DeviceType.entries[action.id.toInt()]))
    }
}
