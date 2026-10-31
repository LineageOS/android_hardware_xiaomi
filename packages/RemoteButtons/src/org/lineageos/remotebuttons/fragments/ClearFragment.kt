/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import android.widget.Toast
import androidx.fragment.app.activityViewModels
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.viewmodels.SetupViewModel

/** Removes the IR codes from the remote. */
class ClearFragment : RemoteButtonsGuidedStepFragment() {

    private val viewModel: SetupViewModel by activityViewModels()

    override fun onCreateGuidance(savedInstanceState: Bundle?) = GuidanceStylist.Guidance(
        getString(R.string.clear_title),
        getString(R.string.clear_description),
        getString(R.string.app_name),
        null,
    )

    override fun onCreateActions(actions: MutableList<GuidedAction>, savedInstanceState: Bundle?) {
        actions += GuidedAction.Builder(requireContext()).id(ACTION_CLEAR).title(R.string.action_clear).build()
        actions += GuidedAction.Builder(requireContext()).clickAction(GuidedAction.ACTION_ID_CANCEL).build()
    }

    override fun onGuidedActionClicked(action: GuidedAction) {
        if (action.id != ACTION_CLEAR) {
            finishStep()
            return
        }
        action.isEnabled = false
        notifyActionChanged(findActionPositionById(ACTION_CLEAR))
        lifecycleScope.launch {
            val message = if (viewModel.clear().isSuccess) R.string.clear_done else R.string.clear_failed
            Toast.makeText(requireContext(), message, Toast.LENGTH_LONG).show()
            finishStep()
        }
    }

    private companion object {
        const val ACTION_CLEAR = 1L
    }
}
