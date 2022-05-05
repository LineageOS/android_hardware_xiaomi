/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package org.ifaa.aidl.manager;

import android.app.KeyguardManager;
import android.app.Service;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.hardware.fingerprint.Fingerprint;
import android.hardware.fingerprint.FingerprintManager;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import org.ifaa.aidl.manager.IfaaManagerService;
import org.json.JSONObject;
import org.json.JSONException;
import vendor.xiaomi.hardware.mlipay.V1_1.IMlipayService;

public class IfaaService extends Service {
    private static final String LOG_TAG = IfaaService.class.getSimpleName();

    private static final int AUTH_TYPE_NOT_SUPPORT = 0;
    private static final int AUTH_TYPE_FINGERPRINT = 1;
    private static final int AUTH_TYPE_IRIS = 1<<1;
    private static final int AUTH_TYPE_OPTICAL_FINGERPRINT = 1<<4;

    private static final int ACTIVITY_START_SUCCESS = 0;
    private static final int ACTIVITY_START_FAILED = -1;

    private IMlipayService mMlipayService = null;

    private final IBinder mIFAABinder = new IfaaServiceStub(this);

    private final class IfaaServiceStub extends IfaaManagerService.Stub {
        IfaaServiceStub(IfaaService ifaaService) {
            new WeakReference(ifaaService);
        }

        @Override
        public int getSupportBIOTypes() {
            int ifaaProp = SystemProperties.getInt("persist.vendor.sys.pay.ifaa", 0);
            String fpVendor = SystemProperties.get("persist.vendor.sys.fp.vendor", "");

            int res = "none".equalsIgnoreCase(fpVendor) ?
                    ifaaProp & AUTH_TYPE_IRIS :
                    ifaaProp & (AUTH_TYPE_FINGERPRINT | AUTH_TYPE_IRIS);

            if ((res & AUTH_TYPE_FINGERPRINT) == AUTH_TYPE_FINGERPRINT &&
                    SystemProperties.getBoolean("ro.hardware.fp.udfps", false)) {
                res |= AUTH_TYPE_OPTICAL_FINGERPRINT;
            }

            return res;
        }

        @Override
        public int startBIOManager(int authType) {
            int res = ACTIVITY_START_FAILED;

            if (authType == AUTH_TYPE_FINGERPRINT) {
                Intent intent = new Intent("android.settings.SECURITY_SETTINGS");
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                getApplicationContext().startActivity(intent);
                res = ACTIVITY_START_SUCCESS;
            }

            return res;
        }

        @Override
        public String getDeviceModel() {
            return Build.MANUFACTURER + "-" + Build.DEVICE;
        }

        @Override
        public byte[] processCmd(byte[] param) {
            IMlipayService mlipayService = getMlipayService();
            if (mlipayService == null) {
                Log.e(LOG_TAG, "Failed to open mlipay HAL");
                return null;
            }

            ArrayList<Byte> paramByteArray = new ArrayList<Byte>();
            for (byte b : param) {
                paramByteArray.add(Byte.valueOf(b));
            }

            byte[] receiveBuffer = null;

            try {
                ArrayList<Byte> receiveBufferByteArray = mlipayService.invoke_command(paramByteArray,
                        paramByteArray.size());

                receiveBuffer = new byte[receiveBufferByteArray.size()];
                for (int i = 0; i < receiveBufferByteArray.size(); i++) {
                    receiveBuffer[i] = receiveBufferByteArray.get(i).byteValue();
                }
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "processCmdImpl: mlipay invoke_command failed", e);
            }

            return receiveBuffer;
        }

        @Override
        public int getVersion() {
            return 4;
        }

        @Override
        public String getExtInfo(int authType, String keyExtInfo) {
            return initExtString();
        }

        @Override
        public void setExtInfo(int authType, String keyExtInfo, String valExtInfo) {
            // nothing
        }

        @Override
        public int getEnabled(int bioType) {
            return 1 == bioType ? 1000 : 1003;
        }

        @Override
        public int[] getIDList(int bioType) {
            int[] idList = new int[0];

            IMlipayService mlipayService = getMlipayService();
            if (mlipayService == null) {
                Log.e(LOG_TAG, "getIDListImpl: Failed to open mlipay HAL");
                return idList;
            }

            try {
                ArrayList<Integer> idListAL = mlipayService.ifaa_get_idlist(bioType);
                idList = new int[idListAL.size()];
                for (int i = 0; i < idListAL.size(); i++) {
                    idList[i] = idListAL.get(i).intValue();
                }
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "getIDListImpl: mlipay ifaa_get_idlist failed", e);
            }

            return idList;
        }
    };

    @Override // android.app.Service
    public IBinder onBind(Intent intent) {
        return this.mIFAABinder;
    }

    // Utils

    private IMlipayService getMlipayService() {
        if (mMlipayService == null) {
            try {
                mMlipayService = IMlipayService.getService();
            } catch (RemoteException e) {
                // do nothing
            }
        }

        return mMlipayService;
    }

    private String initExtString() {
        JSONObject obj = new JSONObject();
        JSONObject keyInfo = new JSONObject();
        String xy = SystemProperties.get("persist.vendor.sys.fp.udfps.location.X_Y", "");
        String wh = SystemProperties.get("persist.vendor.sys.fp.udfps.size.width_height", "");
        try {
            if (!validateVal(xy) || !validateVal(wh)) {
                Log.e(LOG_TAG, "initExtString: invalidate, xy: " + xy + ", wh: " + wh);
                return "";
            }
            String[] split = xy.split(",");
            String[] split2 = wh.split(",");
            keyInfo.put("startX", Integer.parseInt(split[0]));
            keyInfo.put("startY", Integer.parseInt(split[1]));
            keyInfo.put("width", Integer.parseInt(split2[0]));
            keyInfo.put("height", Integer.parseInt(split2[1]));
            keyInfo.put("navConflict", true);
            obj.put("type", 0);
            obj.put("fullView", keyInfo);
            return obj.toString();
        } catch (Exception e) {
            Log.e(LOG_TAG, "initExtString: Exception, xy: " + xy + ", wh: " + wh, e);
            return "";
        }
    }

    private boolean validateVal(String str) {
        return !"".equalsIgnoreCase(str) && str.contains(",");
    }
}
