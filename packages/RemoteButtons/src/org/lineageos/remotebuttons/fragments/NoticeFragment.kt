/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import org.lineageos.remotebuttons.R

/** Explains what will happen, and that music will play. */
class NoticeFragment : RemoteButtonsGuidedStepFragment() {

    override fun onCreateGuidance(savedInstanceState: Bundle?) =
        GuidanceStylist.Guidance(
            getString(R.string.notice_title),
            getString(R.string.notice_description),
            getString(R.string.app_name),
            null,
        )

    override fun onCreateActions(actions: MutableList<GuidedAction>, savedInstanceState: Bundle?) {
        actions +=
            GuidedAction.Builder(requireContext())
                .clickAction(GuidedAction.ACTION_ID_CONTINUE)
                .build()
    }

    override fun onGuidedActionClicked(action: GuidedAction) {
        add(parentFragmentManager, DeviceTypeFragment())
    }
}
