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

class WelcomeFragment : RemoteButtonsGuidedStepFragment() {

    private val viewModel: SetupViewModel by activityViewModels()

    override fun onCreateGuidance(savedInstanceState: Bundle?) = GuidanceStylist.Guidance(
        getString(R.string.app_name),
        getString(R.string.welcome_description),
        null,
        requireContext().getDrawable(R.drawable.ic_remote),
    )

    override fun onResume() {
        super.onResume()
        refresh()
    }

    private fun refresh() {
        val repository = viewModel.repository
        guidanceStylist.descriptionView?.text = getString(R.string.welcome_description) + "\n\n" +
            (repository.configured?.let { getString(R.string.status_configured, it) }
                ?: getString(R.string.status_not_configured))
        actions = buildList {
            add(action(ACTION_SETUP, R.string.action_set_up))
            // Always offered: the remote may hold codes set up elsewhere, e.g. by the stock firmware.
            add(action(ACTION_CLEAR, R.string.action_clear))
        }
    }

    private fun action(id: Long, title: Int) = GuidedAction.Builder(requireContext()).id(id).title(title).build()

    override fun onGuidedActionClicked(action: GuidedAction) {
        when (action.id) {
            ACTION_SETUP -> add(parentFragmentManager, NoticeFragment())
            ACTION_CLEAR -> add(parentFragmentManager, ClearFragment())
        }
    }

    private companion object {
        const val ACTION_SETUP = 1L
        const val ACTION_CLEAR = 2L
    }
}
