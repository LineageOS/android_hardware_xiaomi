/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.view.View
import android.view.ViewTreeObserver
import android.widget.ImageView
import android.widget.TextView
import android.os.Bundle
import androidx.leanback.app.GuidedStepSupportFragment

/** Base guided step that restores action focus and avoids Leanback's pane transitions. */
abstract class RemoteButtonsGuidedStepFragment : GuidedStepSupportFragment() {

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        // Correct the guidance position on the first pre-draw, before the restored step is shown.
        view.viewTreeObserver.addOnPreDrawListener(object : ViewTreeObserver.OnPreDrawListener {
            override fun onPreDraw(): Boolean {
                view.viewTreeObserver.removeOnPreDrawListener(this)
                alignGuidanceToKeyline(view)
                return true
            }
        })
    }

    override fun onResume() {
        super.onResume()

        val root = view ?: return
        root.post {
            if (actions.isNotEmpty()) {
                val position = getSelectedActionPosition().takeIf { it in actions.indices } ?: 0
                setSelectedActionPosition(position)
                guidedActionsStylist.actionsGridView?.requestFocus()
            }
        }
    }

    override fun onProvideFragmentTransitions() {
        setEnterTransition(null)
        setExitTransition(null)
        setSharedElementEnterTransition(null)
    }

    private fun alignGuidanceToKeyline(root: View) {
        val guidance = root.findViewById<View>(androidx.leanback.R.id.guidance_container) ?: return
        val title = root.findViewById<TextView>(androidx.leanback.R.id.guidance_title) ?: return
        val breadcrumb = root.findViewById<TextView>(androidx.leanback.R.id.guidance_breadcrumb) ?: return

        val attrs = requireContext().obtainStyledAttributes(
            intArrayOf(androidx.leanback.R.attr.guidedStepKeyline),
        )
        val keylinePercent = attrs.getFloat(0, DEFAULT_KEYLINE_PERCENT)
        attrs.recycle()

        val keyline = (guidance.height * keylinePercent / 100f).toInt()
        val offset = keyline - title.baseline - breadcrumb.height - title.paddingTop - breadcrumb.top
        breadcrumb.offsetTopAndBottom(offset)
        title.offsetTopAndBottom(offset)

        root.findViewById<TextView>(androidx.leanback.R.id.guidance_description)
            ?.offsetTopAndBottom(offset)
        root.findViewById<ImageView>(androidx.leanback.R.id.guidance_icon)?.let { icon ->
            if (icon.drawable != null) {
                icon.offsetTopAndBottom(keyline - icon.height / 2 - icon.top)
            }
        }
    }

    private companion object {
        const val DEFAULT_KEYLINE_PERCENT = 40f
    }
}
