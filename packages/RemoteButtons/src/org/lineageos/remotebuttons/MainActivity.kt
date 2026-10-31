/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotebuttons

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.fragment.app.FragmentActivity
import androidx.leanback.app.GuidedStepSupportFragment
import androidx.lifecycle.lifecycleScope
import kotlinx.coroutines.launch
import org.lineageos.remotebuttons.fragments.ClearFragment
import org.lineageos.remotebuttons.fragments.NoticeFragment
import org.lineageos.remotebuttons.fragments.WelcomeFragment
import org.lineageos.remotebuttons.remote.RemoteProgrammers

class MainActivity : FragmentActivity() {

    private val requestBluetooth = registerForActivityResult(ActivityResultContracts.RequestPermission()) {
        if (!it) {
            Toast.makeText(this, R.string.permission_needed, Toast.LENGTH_LONG).show()
            finish()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        if (checkSelfPermission(Manifest.permission.BLUETOOTH_CONNECT) != PackageManager.PERMISSION_GRANTED) {
            requestBluetooth.launch(Manifest.permission.BLUETOOTH_CONNECT)
        }
        if (savedInstanceState == null) {
            // A step opened from the TV Settings page is the root, so leaving it returns there.
            val root = startStep(intent)
            GuidedStepSupportFragment.addAsRoot(this, root ?: WelcomeFragment(), android.R.id.content)
            if (root == null) openStart(intent)
        }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        openStart(intent)
    }

    /** Opens the step requested by the TV Settings page, if any. */
    private fun openStart(intent: Intent) {
        if (intent.getStringExtra(EXTRA_START) == START_PROBE) {
            // Diagnostics for porting: `am start -n <package>/.MainActivity --es start probe`.
            lifecycleScope.launch { RemoteProgrammers(this@MainActivity).describe() }
            return
        }
        val step = startStep(intent) ?: return
        GuidedStepSupportFragment.add(supportFragmentManager, step)
    }

    private fun startStep(intent: Intent): GuidedStepSupportFragment? =
        when (intent.getStringExtra(EXTRA_START)) {
            START_SETUP -> NoticeFragment()
            START_CLEAR -> ClearFragment()
            else -> null
        }

    companion object {
        /** Step to open from the TV Settings page: [START_SETUP] or [START_CLEAR]. */
        const val EXTRA_START = "start"
        const val START_SETUP = "setup"
        const val START_CLEAR = "clear"
        const val START_PROBE = "probe"
    }
}
