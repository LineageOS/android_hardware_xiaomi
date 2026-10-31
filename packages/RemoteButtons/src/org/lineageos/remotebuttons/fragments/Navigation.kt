/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import androidx.leanback.app.GuidedStepSupportFragment

/** Leaves this step: back to the previous one, or out of the app if it was opened on its own. */
fun GuidedStepSupportFragment.finishStep() {
    if (parentFragmentManager.backStackEntryCount > 0) {
        parentFragmentManager.popBackStack()
    } else {
        requireActivity().finish()
    }
}
