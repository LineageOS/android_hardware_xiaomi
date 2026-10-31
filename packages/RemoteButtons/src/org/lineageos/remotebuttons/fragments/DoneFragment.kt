/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import org.lineageos.remotebuttons.R

class DoneFragment : RemoteButtonsGuidedStepFragment() {

    override fun onCreateGuidance(savedInstanceState: Bundle?) =
        GuidanceStylist.Guidance(
            getString(R.string.done_title, requireArguments().getString(ARG_BRAND)),
            getString(R.string.done_description),
            getString(R.string.app_name),
            requireContext().getDrawable(R.drawable.ic_remote),
        )

    override fun onCreateActions(actions: MutableList<GuidedAction>, savedInstanceState: Bundle?) {
        actions +=
            GuidedAction.Builder(requireContext())
                .clickAction(GuidedAction.ACTION_ID_FINISH)
                .build()
    }

    override fun onGuidedActionClicked(action: GuidedAction) {
        requireActivity().finish()
    }

    companion object {
        private const val ARG_BRAND = "brand"

        fun newInstance(brand: String) =
            DoneFragment().apply { arguments = Bundle().apply { putString(ARG_BRAND, brand) } }
    }
}
