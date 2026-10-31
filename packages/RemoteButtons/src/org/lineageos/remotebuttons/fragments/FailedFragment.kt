/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import androidx.fragment.app.activityViewModels
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.viewmodels.SetupViewModel

/** None of the brand's code sets worked. */
class FailedFragment : RemoteButtonsGuidedStepFragment() {

    private val viewModel: SetupViewModel by activityViewModels()

    override fun onCreateGuidance(savedInstanceState: Bundle?) = GuidanceStylist.Guidance(
        getString(R.string.failed_title, requireArguments().getString(ARG_BRAND)),
        getString(R.string.failed_description),
        getString(R.string.app_name),
        null,
    )

    override fun onCreateActions(actions: MutableList<GuidedAction>, savedInstanceState: Bundle?) {
        actions += GuidedAction.Builder(requireContext()).id(ACTION_RETRY).title(R.string.action_try_again).build()
        actions += GuidedAction.Builder(requireContext()).id(ACTION_OTHER_BRAND).title(R.string.action_other_brand).build()
        actions += GuidedAction.Builder(requireContext()).clickAction(GuidedAction.ACTION_ID_CANCEL).build()
    }

    override fun onGuidedActionClicked(action: GuidedAction) {
        when (action.id) {
            ACTION_RETRY -> {
                parentFragmentManager.popBackStack()
                viewModel.restart()
            }

            ACTION_OTHER_BRAND -> popBackStackToGuidedStepSupportFragment(BrandFragment::class.java, 0)
            else -> requireActivity().finish()
        }
    }

    companion object {
        private const val ARG_BRAND = "brand"
        private const val ACTION_RETRY = 1L
        private const val ACTION_OTHER_BRAND = 2L

        fun newInstance(brand: String) = FailedFragment().apply { arguments = Bundle().apply { putString(ARG_BRAND, brand) } }
    }
}
