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
import java.util.ArrayList;
import java.util.List;
import org.ifaa.aidl.manager.IfaaManagerService;
import org.json.JSONObject;
import org.json.JSONException;
import vendor.xiaomi.hardware.mlipay.V1_1.IMlipayService;

public class IfaaService extends Service {
    private static final String LOG_TAG = "IfaaService";

    private static final int AUTH_TYPE_NOT_SUPPORT = 0;
    private static final int AUTH_TYPE_FINGERPRINT = 1;
    private static final int AUTH_TYPE_IRIS = 1<<1;
    private static final int AUTH_TYPE_OPTICAL_FINGERPRINT = 1<<4;

    private static final int ACTIVITY_START_SUCCESS = 0;
    private static final int ACTIVITY_START_FAILED = -1;

    private IMlipayService mMlipayService = null;
    private FingerprintManager mFingerprintManager;

    static boolean sIsFod = SystemProperties.getBoolean("ro.hardware.fp.fod", false);

    private IfaaManagerService.Stub mBinder = new IfaaManagerService.Stub() {
        @Override
        public int getSupportBIOTypes() {
            return IfaaService.this.getSupportBIOTypesImpl();
        }

        @Override
        public int startBIOManager(int authType) {
            return IfaaService.this.startBIOManagerImpl(authType);
        }

        @Override
        public String getDeviceModel() {
            return IfaaService.this.getDeviceModelImpl();
        }

        @Override
        public byte[] processCmd(byte[] param) {
            return IfaaService.this.processCmdImpl(param);
        }

        @Override
        public int getVersion() {
            return IfaaService.this.getVersionImpl();
        }

        @Override
        public String getExtInfo(int authType, String keyExtInfo) {
            return IfaaService.this.getExtInfoImpl(authType, keyExtInfo);
        }

        @Override
        public void setExtInfo(int authType, String keyExtInfo, String valExtInfo) {
            IfaaService.this.setExtInfoImpl(authType, keyExtInfo, valExtInfo);
        }

        @Override
        public int getEnabled(int bioType) {
            return IfaaService.this.getEnabledImpl(bioType);
        }

        @Override
        public int[] getIDList(int bioType) {
            return IfaaService.this.getIDListImpl(bioType);
        }
    };

    @Override // android.app.Service
    public IBinder onBind(Intent intent) {
        return this.mBinder;
    }

    private byte[] processCmdImpl(byte[] param) {
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

    public int getSupportBIOTypesImpl() {
        int ifaaProp = SystemProperties.getInt("persist.vendor.sys.pay.ifaa", 0);
        String fpVendor = SystemProperties.get("persist.vendor.sys.fp.vendor", "");

        int res = fpVendor.equals("") ?
                ifaaProp & AUTH_TYPE_IRIS :
                ifaaProp & (AUTH_TYPE_FINGERPRINT | AUTH_TYPE_IRIS);

        if ((res & AUTH_TYPE_FINGERPRINT) == AUTH_TYPE_FINGERPRINT && sIsFod) {
            res |= AUTH_TYPE_OPTICAL_FINGERPRINT;
        }

        return res;
    }

    public int startBIOManagerImpl(int authType) {
        int res = ACTIVITY_START_FAILED;

        if (authType == AUTH_TYPE_FINGERPRINT) {
            Intent intent = new Intent("android.settings.SECURITY_SETTINGS");
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            getApplicationContext().startActivity(intent);
            res = ACTIVITY_START_SUCCESS;
        }

        return res;
    }

    public String getDeviceModelImpl() {
        return Build.MANUFACTURER + "-" + Build.DEVICE;
    }

    public int getVersionImpl() {
        return 4;
    }

    public String getExtInfoImpl(int authType, String keyExtInfo) {
        if ("org.ifaa.ext.key.GET_SENSOR_LOCATION".equals(keyExtInfo)) {
            if (!sIsFod) {
                Log.i(LOG_TAG, "getExtInfo: Requested sensor location but not a FOD device");
                return "";
            }

            int[] fodProps = parseFodProps();
            if (fodProps == null) {
                Log.e(LOG_TAG, "getExtInfo: Couldn't get FOD properties");
                return "";
            }

            JSONObject obj = new JSONObject();
            JSONObject keyInfo = new JSONObject();

            try {
                keyInfo.put("startX", fodProps[0]);
                keyInfo.put("startY", fodProps[1]);
                keyInfo.put("width", fodProps[2]);
                keyInfo.put("height", fodProps[3]);
                keyInfo.put("navConflict", true);

                obj.put("type", 0);
                obj.put("fullView", keyInfo);
            } catch (JSONException e) {
                Log.e(LOG_TAG, "getExtInfo: JSONException", e);
                return "";
            }

            return obj.toString();
        }

        Log.e(LOG_TAG, "getExtInfo: Didn't request supported ext info");
        return "";
    }

    public void setExtInfoImpl(int authType, String keyExtInfo, String valExtInfo) {
        // nothing
    }

    public int getEnabledImpl(int bioType) {
        KeyguardManager keyguardManager = (KeyguardManager) getSystemService("keyguard");

        if (!keyguardManager.isDeviceSecure()) {
            return 1003;
        }
        if (bioType == 4) {
            return 1000;
        }
        if (bioType != 1) {
            return 1001;
        }

        FingerprintManager fingerprintManager = getFingerprintManager();

        if (fingerprintManager == null) {
            return 1001;
        }
        if (!fingerprintManager.hasEnrolledFingerprints()) {
            return 1002;
        }
        return 1000;
    }

    public int[] getIDListImpl(int bioType) {
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

    private FingerprintManager getFingerprintManager() {
        if (mFingerprintManager == null) {
            Context applicationContext = getApplicationContext();

            if (!getApplicationContext().getPackageManager().hasSystemFeature("android.hardware.fingerprint"))
                return null;
    
            mFingerprintManager = (FingerprintManager) applicationContext.getSystemService("fingerprint");
        }

        return mFingerprintManager;
    }

    private boolean validateVal(String value) {
        return !"".equalsIgnoreCase(value) && value.contains(",");
    }

    private int[] parseFodProps() {
        String xy = SystemProperties.get("persist.vendor.sys.fp.fod.location.X_Y", "");
        String wh = SystemProperties.get("persist.vendor.sys.fp.fod.size.width_height", "");

        if (!validateVal(xy) || !validateVal(wh)) {
            Log.e(LOG_TAG, "Couldn't get FOD location and size properties");
            return null;
        }

        String[] splitXy;
        String[] splitWh;
        try {
            splitXy = xy.split(",");
            splitWh = wh.split(",");
        } catch (Exception e) {
            Log.e(LOG_TAG, "Couldn't split FOD location and size properties");
            return null;
        }

        if (splitXy.length != 2 || splitWh.length != 2) {
            Log.e(LOG_TAG, "Unexpected FOD location and size properties split length");
            return null;
        }

        try {
            return new int[] {
                Integer.parseInt(splitXy[0]),
                Integer.parseInt(splitXy[1]),
                Integer.parseInt(splitWh[0]),
                Integer.parseInt(splitWh[1]),
            };
        } catch (Exception e) {
            Log.e(LOG_TAG, "Couldn't parse FOD location and size properties");
            return null;
        }
    }
}
