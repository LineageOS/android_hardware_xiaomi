/*
 * SPDX-FileCopyrightText: 2022-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.ifaa.aidl.manager

import android.app.Service
import android.content.Intent
import android.os.Build
import android.os.IBinder
import android.os.IHwBinder
import android.os.ServiceManager
import android.os.SystemProperties
import android.provider.Settings
import android.util.Log
import java.util.concurrent.FutureTask
import java.util.concurrent.TimeUnit
import java.util.concurrent.TimeoutException
import org.json.JSONObject
import vendor.xiaomi.hardware.mlipay.IMlipayService as AidlMlipayService
import vendor.xiaomi.hardware.mlipay.V1_1.IMlipayService as HidlMlipayService

class IfaaService : Service() {
    private var _aidlMlipayService: AidlMlipayService? = null
    private var _aidlMlipayServiceLookup: FutureTask<IBinder?>? = null
    private var _hidlMlipayService: HidlMlipayService? = null
    private val isAidlMlipayDeclared by lazy { ServiceManager.isDeclared(AIDL_MLIPAY_SERVICE_NAME) }

    private val mBinder =
        object : IfaaManagerService.Stub() {
            override fun getSupportBIOTypes(): Int {
                val fpVendor = SystemProperties.get(FP_VENDOR_PROP, "")
                val isUdfps = SystemProperties.getBoolean(IS_UDFPS_PROP, false)

                val supportedBioMask =
                    when (!invalidFpVendors.contains(fpVendor.lowercase())) {
                        true -> AUTH_TYPE_FINGERPRINT or AUTH_TYPE_IRIS
                        else -> AUTH_TYPE_IRIS
                    }

                val ifaaProp =
                    SystemProperties.getInt(SUPPORTED_BIO_MASK_PROP, 0) and
                        supportedBioMask or
                        when (isUdfps) {
                            true -> AUTH_TYPE_OPTICAL_FINGERPRINT
                            else -> 0
                        }

                return ifaaProp
            }

            override fun startBIOManager(authType: Int) =
                when (authType) {
                    AUTH_TYPE_FINGERPRINT -> {
                        applicationContext.startActivity(
                            Intent(Settings.ACTION_SECURITY_SETTINGS).apply {
                                addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                            }
                        )

                        COMMAND_OK
                    }

                    else -> COMMAND_FAIL
                }

            override fun getDeviceModel() = "${Build.MANUFACTURER}-${Build.DEVICE}"

            override fun processCmd(param: ByteArray): ByteArray? {
                try {
                    getAidlMlipayService()?.let {
                        return it.invoke_command(param, param.size)
                    }

                    return getHidlMlipayService()
                        ?.invoke_command(ArrayList(param.toList()), param.size)
                        ?.toByteArray()
                } catch (e: Exception) {
                    Log.e(LOG_TAG, "processCmdImpl: mlipay invoke_command failed", e)
                    return null
                }
            }

            override fun getVersion() = 4

            override fun getExtInfo(authType: Int, keyExtInfo: String) = initExtString()

            override fun setExtInfo(authType: Int, keyExtInfo: String, valExtInfo: String) {
                // Do nothing
            }

            override fun getEnabled(bioType: Int) =
                when (bioType) {
                    AUTH_TYPE_FINGERPRINT -> BIOMETRICS_AVAILABLE
                    else -> SCREEN_LOCK_NONE
                }

            override fun getIDList(bioType: Int): IntArray {
                try {
                    getAidlMlipayService()?.let {
                        return it.ifaa_get_idlist(bioType)
                    }

                    return getHidlMlipayService()?.ifaa_get_idlist(bioType)?.toIntArray()
                        ?: IntArray(0)
                } catch (e: Exception) {
                    Log.e(LOG_TAG, "getIDListImpl: mlipay ifaa_get_idlist failed", e)
                    return IntArray(0)
                }
            }
        }

    override fun onBind(intent: Intent) = mBinder

    @Synchronized
    private fun getAidlMlipayService(): AidlMlipayService? {
        _aidlMlipayService?.let {
            if (it.asBinder().isBinderAlive) return it
            _aidlMlipayService = null
        }

        try {
            if (!isAidlMlipayDeclared) return null

            // Reuse the pending lookup if lazy HAL startup exceeds the timeout.
            val lookup =
                _aidlMlipayServiceLookup
                    ?: FutureTask<IBinder?> {
                            ServiceManager.waitForDeclaredService(AIDL_MLIPAY_SERVICE_NAME)
                        }
                        .also {
                            _aidlMlipayServiceLookup = it
                            Thread(it, "IFAA-mlipay").apply { isDaemon = true }.start()
                        }
            val binder = lookup.get(MLIPAY_SERVICE_TIMEOUT_MS, TimeUnit.MILLISECONDS)
            _aidlMlipayServiceLookup = null
            val mlipayBinder =
                binder
                    ?: run {
                        Log.w(LOG_TAG, "AIDL mlipay service unavailable, trying HIDL")
                        return null
                    }
            val service = AidlMlipayService.Stub.asInterface(mlipayBinder) ?: return null
            mlipayBinder.linkToDeath(
                IBinder.DeathRecipient {
                    synchronized(this) {
                        if (_aidlMlipayService?.asBinder() == mlipayBinder) {
                            Log.i(LOG_TAG, "AIDL mlipay service died")
                            _aidlMlipayService = null
                        }
                    }
                },
                0,
            )
            _aidlMlipayService = service
            Log.i(LOG_TAG, "Connected to AIDL mlipay service")
            return service
        } catch (e: TimeoutException) {
            Log.w(LOG_TAG, "AIDL mlipay service unavailable, trying HIDL")
            return null
        } catch (e: InterruptedException) {
            Thread.currentThread().interrupt()
            Log.w(LOG_TAG, "Interrupted while connecting to AIDL mlipay service", e)
            return null
        } catch (e: Exception) {
            _aidlMlipayServiceLookup = null
            Log.w(LOG_TAG, "Unable to connect to AIDL mlipay service, trying HIDL", e)
            return null
        }
    }

    @Synchronized
    private fun getHidlMlipayService(): HidlMlipayService? {
        _hidlMlipayService?.let {
            return it
        }

        try {
            // Avoid waiting for HIDL on AIDL-only devices.
            val service = HidlMlipayService.getService(false) ?: return null
            val linked =
                service.linkToDeath(
                    IHwBinder.DeathRecipient {
                        synchronized(this) {
                            if (_hidlMlipayService?.asBinder() == service.asBinder()) {
                                Log.i(LOG_TAG, "HIDL mlipay service died")
                                _hidlMlipayService = null
                            }
                        }
                    },
                    0,
                )
            if (!linked) return null
            _hidlMlipayService = service
            Log.i(LOG_TAG, "Connected to HIDL mlipay service")
            return service
        } catch (e: Exception) {
            Log.w(LOG_TAG, "Unable to connect to HIDL mlipay service", e)
            return null
        }
    }

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

        private const val AIDL_MLIPAY_SERVICE_NAME =
            "vendor.xiaomi.hardware.mlipay.IMlipayService/default"
        private const val MLIPAY_SERVICE_TIMEOUT_MS = 2000L

        private const val SUPPORTED_BIO_MASK_PROP = "persist.vendor.sys.pay.ifaa"
        private const val FP_VENDOR_PROP = "persist.vendor.sys.fp.vendor"
        private const val IS_UDFPS_PROP = "ro.hardware.fp.udfps"
        private const val UDFPS_LOCATION_X_Y_PROP = "persist.vendor.sys.fp.udfps.location.X_Y"
        private const val UDFPS_SIZE_W_H_PROP = "persist.vendor.sys.fp.udfps.size.width_height"

        private val invalidFpVendors = arrayOf("", "none")
    }
}
