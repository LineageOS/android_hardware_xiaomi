/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import android.view.View
import androidx.fragment.app.activityViewModels
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.repeatOnLifecycle
import kotlinx.coroutines.launch
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.database.Brand
import org.lineageos.remotebuttons.ir.DeviceType
import org.lineageos.remotebuttons.viewmodels.SetupViewModel
import org.lineageos.remotebuttons.viewmodels.SetupViewModel.Pairing

/**
 * Tries the brand's code sets one by one: each is sent to the remote, then a chime plays and the
 * user tells whether the remote's volume buttons change the TV volume.
 */
class PairingFragment : RemoteButtonsGuidedStepFragment() {

    private val viewModel: SetupViewModel by activityViewModels()
    private val brand by lazy {
        Brand(requireArguments().getString(ARG_BRAND_ID)!!, requireArguments().getString(ARG_BRAND_NAME)!!)
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (savedInstanceState == null) {
            viewModel.startPairing(DeviceType.entries[requireArguments().getInt(ARG_TYPE)], brand)
        }
    }

    override fun onCreateGuidance(savedInstanceState: Bundle?) = GuidanceStylist.Guidance(
        getString(R.string.pairing_programming),
        null,
        brand.name,
        null,
    )

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        viewLifecycleOwner.lifecycleScope.launch {
            viewLifecycleOwner.repeatOnLifecycle(Lifecycle.State.STARTED) {
                viewModel.pairing.collect(::render)
            }
        }
    }

    private fun render(state: Pairing) {
        val title = guidanceStylist.titleView
        val description = guidanceStylist.descriptionView
        when (state) {
            Pairing.Idle -> Unit

            is Pairing.Programming -> {
                title?.setText(R.string.pairing_programming)
                description?.text = attempt(state.index, state.count)
                actions = emptyList()
            }

            is Pairing.Ready -> {
                title?.setText(R.string.pairing_title)
                description?.text = getString(R.string.pairing_description) + "\n\n" + attempt(state.index, state.count)
                actions = listOf(
                    action(ACTION_YES, R.string.action_yes),
                    action(ACTION_NO, R.string.action_no),
                    GuidedAction.Builder(requireContext())
                        .id(ACTION_CHOOSE)
                        .title(R.string.action_choose_code_set)
                        .subActions(
                            List(state.count) { i ->
                                GuidedAction.Builder(requireContext())
                                    .id(i.toLong())
                                    .title(getString(R.string.code_set_item, i + 1))
                                    .build()
                            },
                        )
                        .build(),
                )
            }

            is Pairing.Error -> {
                title?.setText(R.string.pairing_error_title)
                description?.text = getString(R.string.pairing_error_description, state.message) + "\n\n" +
                    attempt(state.index, state.count)
                actions = listOf(action(ACTION_RETRY, R.string.action_try_again), action(ACTION_NO, R.string.action_skip))
            }

            Pairing.Exhausted -> add(parentFragmentManager, FailedFragment.newInstance(brand.name))
        }
    }

    private fun attempt(index: Int, count: Int) = getString(R.string.pairing_attempt, index + 1, count)

    private fun action(id: Long, title: Int) = GuidedAction.Builder(requireContext()).id(id).title(title).build()

    override fun onGuidedActionClicked(action: GuidedAction) {
        when (action.id) {
            ACTION_YES -> {
                viewModel.confirm()
                add(parentFragmentManager, DoneFragment.newInstance(brand.name))
            }

            ACTION_NO -> viewModel.next()
            ACTION_RETRY -> viewModel.retry()
        }
    }

    override fun onSubGuidedActionClicked(action: GuidedAction): Boolean {
        viewModel.pick(action.id.toInt())
        return true
    }

    override fun onStop() {
        super.onStop()
        // Leaving the step (back, or the app going away) stops the chime and any programming.
        if (!requireActivity().isChangingConfigurations) viewModel.stopPairing()
    }

    companion object {
        private const val ARG_TYPE = "type"
        private const val ARG_BRAND_ID = "brand_id"
        private const val ARG_BRAND_NAME = "brand_name"
        private const val ACTION_YES = 1L
        private const val ACTION_NO = 2L
        private const val ACTION_CHOOSE = 3L
        private const val ACTION_RETRY = 4L

        fun newInstance(type: DeviceType, brand: Brand) = PairingFragment().apply {
            arguments = Bundle().apply {
                putInt(ARG_TYPE, type.ordinal)
                putString(ARG_BRAND_ID, brand.id)
                putString(ARG_BRAND_NAME, brand.name)
            }
        }
    }
}
