/*
 * SPDX-FileCopyrightText: 2022-2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.ifaa.aidl.manager

import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.IHwBinder
import android.os.SystemProperties
import android.provider.Settings
import android.util.Log
import org.ifaa.aidl.manager.IfaaManagerService
import org.json.JSONObject
import vendor.xiaomi.hardware.mlipay.V1_1.IMlipayService

class IfaaService : Service() {
    private var _mlipayService: IMlipayService? = null

    private val mlipayServiceDeathRecipient = IHwBinder.DeathRecipient {
        Log.i(LOG_TAG, "mlipay service died")
        _mlipayService = null
    }

    private val mBinder = object : IfaaManagerService.Stub() {
        override fun getSupportBIOTypes(): Int {
            val fpVendor = SystemProperties.get(FP_VENDOR_PROP, "")
            val isUdfps = SystemProperties.getBoolean(IS_UDFPS_PROP, false)

            val supportedBioMask = when (!invalidFpVendors.contains(fpVendor.lowercase())) {
                true -> AUTH_TYPE_FINGERPRINT or AUTH_TYPE_IRIS
                else -> AUTH_TYPE_IRIS
            }

            val ifaaProp = SystemProperties.getInt(
                SUPPORTED_BIO_MASK_PROP, 0
            ) and supportedBioMask or when (isUdfps) {
                true -> AUTH_TYPE_OPTICAL_FINGERPRINT
                else -> 0
            }

            return ifaaProp
        }

        override fun startBIOManager(authType: Int) = when (authType) {
            AUTH_TYPE_FINGERPRINT -> {
                val intent = Intent(Settings.ACTION_SECURITY_SETTINGS).apply {
                    addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                }

                applicationContext.startActivity(intent)

                COMMAND_OK
            }

            else -> COMMAND_FAIL
        }

        override fun getDeviceModel() = "${Build.MANUFACTURER}-${Build.DEVICE}"

        override fun processCmd(param: ByteArray) = getMlipayService()?.let { mlipayService ->
            var receiveBuffer: ByteArray? = null

            val paramByteArray = ArrayList<Byte>().apply {
                for (byte in param) {
                    add(byte)
                }
            }

            try {
                val receiveBufferByteArray = mlipayService.invoke_command(
                    paramByteArray, paramByteArray.size
                )

                receiveBuffer = receiveBufferByteArray.toByteArray()
            } catch (e: Exception) {
                Log.e(LOG_TAG, "processCmdImpl: mlipay invoke_command failed", e)
            }

            receiveBuffer
        }

        override fun getVersion() = 4

        override fun getExtInfo(authType: Int, keyExtInfo: String) = initExtString()

        override fun setExtInfo(authType: Int, keyExtInfo: String, valExtInfo: String) {
            // Do nothing
        }

        override fun getEnabled(bioType: Int) = when (bioType) {
            AUTH_TYPE_FINGERPRINT -> BIOMETRICS_AVAILABLE
            else -> SCREEN_LOCK_NONE
        }

        override fun getIDList(bioType: Int): IntArray {
            var idList = IntArray(0)

            getMlipayService()?.let { mlipayService ->
                try {
                    val idListAL = mlipayService.ifaa_get_idlist(bioType)

                    idList = idListAL.toIntArray()
                } catch (e: Exception) {
                    Log.e(LOG_TAG, "getIDListImpl: mlipay ifaa_get_idlist failed", e)
                }
            }

            return idList
        }
    }

    override fun onBind(intent: Intent) = mBinder

    private fun getMlipayService() = _mlipayService ?: runCatching {
        IMlipayService.getService(true)
    }.onSuccess {
        _mlipayService = it
        it.linkToDeath(mlipayServiceDeathRecipient, 0)
    }.getOrNull()

    private fun initExtString(): String {
        val obj = JSONObject()
        val keyInfo = JSONObject()

        val xy = SystemProperties.get(UDFPS_LOCATION_X_Y_PROP, "")
        val wh = SystemProperties.get(UDFPS_SIZE_W_H_PROP, "")

        try {
            if (!validateVal(xy) || !validateVal(wh)) {
                Log.e(LOG_TAG, "initExtString: invalidate, xy: $xy, wh: $wh")
                return ""
            }

            val split = xy.split(",")
            val split2 = wh.split(",")

            keyInfo.put("startX", split[0].toInt())
            keyInfo.put("startY", split[1].toInt())
            keyInfo.put("width", split2[0].toInt())
            keyInfo.put("height", split2[1].toInt())
            keyInfo.put("navConflict", true)

            obj.put("type", 0)
            obj.put("fullView", keyInfo)

            return obj.toString()
        } catch (e: Exception) {
            Log.e(LOG_TAG, "initExtString: Exception, xy: $xy, wh: $wh", e)
            return ""
        }
    }

    private fun validateVal(str: String) = !"".equals(str, ignoreCase = true) && str.contains(",")

    companion object {
        private val LOG_TAG = IfaaService::class.simpleName!!

        private const val AUTH_TYPE_NOT_SUPPORT = 0
        private const val AUTH_TYPE_FINGERPRINT = 1
        private const val AUTH_TYPE_IRIS = 1.shl(1)
        private const val AUTH_TYPE_OPTICAL_FINGERPRINT = 1.shl(2)

        private const val BIOMETRICS_AVAILABLE = 1000
        private const val SCREEN_LOCK_NONE = 1003

        private const val COMMAND_OK = 0
        private const val COMMAND_FAIL = -1

        private const val SUPPORTED_BIO_MASK_PROP = "persist.vendor.sys.pay.ifaa"
        private const val FP_VENDOR_PROP = "persist.vendor.sys.fp.vendor"
        private const val IS_UDFPS_PROP = "ro.hardware.fp.udfps"
        private const val UDFPS_LOCATION_X_Y_PROP = "persist.vendor.sys.fp.udfps.location.X_Y"
        private const val UDFPS_SIZE_W_H_PROP = "persist.vendor.sys.fp.udfps.size.width_height"

        private val invalidFpVendors = arrayOf(
            "",
            "none",
        )
    }
}
