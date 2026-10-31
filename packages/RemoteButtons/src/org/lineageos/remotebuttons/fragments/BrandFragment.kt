/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons.fragments

import android.os.Bundle
import androidx.fragment.app.activityViewModels
import androidx.leanback.widget.GuidanceStylist
import androidx.leanback.widget.GuidedAction
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.lineageos.remotebuttons.R
import org.lineageos.remotebuttons.database.Brand
import org.lineageos.remotebuttons.ir.DeviceType
import org.lineageos.remotebuttons.viewmodels.SetupViewModel

/**
 * Asks for the brand. Long lists are grouped by first letter, each letter expanding to its brands,
 * with the most common brands listed first.
 */
class BrandFragment : RemoteButtonsGuidedStepFragment() {

    private val viewModel: SetupViewModel by activityViewModels()
    private val type by lazy { DeviceType.entries[requireArguments().getInt(ARG_TYPE)] }
    private val brands = mutableListOf<Brand>()

    override fun onCreateGuidance(savedInstanceState: Bundle?) =
        GuidanceStylist.Guidance(
            getString(
                when (type) {
                    DeviceType.TV -> R.string.brand_title_tv
                    DeviceType.SOUNDBAR -> R.string.brand_title_soundbar
                    DeviceType.RECEIVER -> R.string.brand_title_receiver
                }
            ),
            null,
            getString(R.string.app_name),
            null,
        )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        lifecycleScope.launch {
            val all = viewModel.brands(type)
            val popular = viewModel.popularBrands(type)
            brands += all
            actions =
                if (all.size <= GROUP_THRESHOLD) {
                    all.map { brandAction(it) }
                } else {
                    popular.map { brandAction(it) } +
                        all.groupBy { initial(it) }
                            .map { (letter, group) ->
                                GuidedAction.Builder(requireContext())
                                    .id(NO_ID)
                                    .title(letter)
                                    .description(
                                        group.joinToString { it.name }.take(DESCRIPTION_LENGTH)
                                    )
                                    .subActions(group.map { brandAction(it) })
                                    .build()
                            }
                }
        }
    }

    private fun brandAction(brand: Brand) =
        GuidedAction.Builder(requireContext())
            .id(brands.indexOf(brand).toLong())
            .title(brand.name)
            .build()

    private fun initial(brand: Brand) =
        brand.name.first().uppercaseChar().let { if (it in 'A'..'Z') "$it" else "#" }

    override fun onGuidedActionClicked(action: GuidedAction) = pick(action)

    override fun onSubGuidedActionClicked(action: GuidedAction): Boolean {
        pick(action)
        return true
    }

    private fun pick(action: GuidedAction) {
        val brand = brands.getOrNull(action.id.toInt()) ?: return
        add(parentFragmentManager, PairingFragment.newInstance(type, brand))
    }

    companion object {
        private const val ARG_TYPE = "type"
        private const val NO_ID = -1L
        private const val GROUP_THRESHOLD = 40
        private const val DESCRIPTION_LENGTH = 60

        fun newInstance(type: DeviceType) =
            BrandFragment().apply { arguments = Bundle().apply { putInt(ARG_TYPE, type.ordinal) } }
    }
}
