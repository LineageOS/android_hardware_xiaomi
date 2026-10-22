/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.tencent.soter.soterserver;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.util.Log;

import java.lang.ref.WeakReference;

public class SoterService extends Service {
    private static final String TAG = "SoterService";

    private final IBinder mBinder = new ServiceStub(this);

    @Override
    public void onCreate() {
        Log.v(TAG, "OnCreate");
        super.onCreate();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.v(TAG, "OnBind");
        return mBinder;
    }

    @Override
    public void onDestroy() {
        Log.v(TAG, "onDestroy");
        super.onDestroy();
    }

    private static boolean isFod() {
        String fpType = SystemProperties.get("persist.vendor.fingerprint.type", "");
        if (fpType.startsWith("udfps")) {
            return true;
        }
        return SystemProperties.getBoolean("ro.hardware.fp.udfps", false);
    }

    public static class UserAppUninstallReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive in");
            if (Intent.ACTION_PACKAGE_FULLY_REMOVED.equals(intent.getAction())) {
                int uid = intent.getIntExtra(Intent.EXTRA_UID, 0);
                boolean isReplacing = intent.getBooleanExtra(Intent.EXTRA_REPLACING, false);
                Log.i(TAG, "install: isReplacing = " + isReplacing);
                Log.i(TAG, "install: uid = " + uid);
                if (!isReplacing) {
                    Log.i(TAG, "begin to use uid to remove ask");
                    try {
                        SoterTZService soter = SoterTZService.get();
                        if (soter != null && soter.isAvailable()) {
                            Log.i(TAG, "start removeAllAuthKey");
                            soter.removeAllUidKey(uid);
                        } else {
                            Log.e(TAG, "soterService is null or unavailable");
                        }
                    } catch (Exception e) {
                        Log.e(TAG, e.toString());
                    }
                }
            }
        }
    }

    private static class ServiceStub extends ISoterService.Stub {
        private final WeakReference<SoterService> mService;

        ServiceStub(SoterService service) {
            mService = new WeakReference<>(service);
        }

        @Override
        public int generateAppSecureKey(int uid) throws RemoteException {
            return SoterTZService.get().generateAppSecureKey(Binder.getCallingUid());
        }

        @Override
        public SoterExportResult getAppSecureKey(int uid) throws RemoteException {
            return SoterTZService.get().getAppSecureKey(Binder.getCallingUid());
        }

        @Override
        public boolean hasAskAlready(int uid) throws RemoteException {
            return SoterTZService.get().hasAskAlready(Binder.getCallingUid());
        }

        @Override
        public int generateAuthKey(int uid, String kname) throws RemoteException {
            return SoterTZService.get().generateAuthKey(Binder.getCallingUid(), kname);
        }

        @Override
        public int removeAuthKey(int uid, String kname) throws RemoteException {
            return SoterTZService.get().removeAuthKey(Binder.getCallingUid(), kname);
        }

        @Override
        public SoterExportResult getAuthKey(int uid, String kname) throws RemoteException {
            return SoterTZService.get().getAuthKey(Binder.getCallingUid(), kname);
        }

        @Override
        public int removeAllAuthKey(int uid) throws RemoteException {
            return SoterTZService.get().removeAllAuthKey(Binder.getCallingUid());
        }

        @Override
        public boolean hasAuthKey(int uid, String kname) throws RemoteException {
            return SoterTZService.get().hasAuthKey(Binder.getCallingUid(), kname);
        }

        @Override
        public SoterSessionResult initSigh(int uid, String kname, String challenge)
                throws RemoteException {
            return SoterTZService.get().initSign(Binder.getCallingUid(), kname, challenge);
        }

        @Override
        public SoterSignResult finishSign(long signSession) throws RemoteException {
            return SoterTZService.get().finishSign(signSession);
        }

        @Override
        public SoterDeviceResult getDeviceId() throws RemoteException {
            return SoterTZService.get().getDeviceId();
        }

        @Override
        public int getVersion() throws RemoteException {
            Log.i(TAG, "getVersion in: 2");
            return 2;
        }

        @Override
        public SoterExtraParam getExtraParam(String key) throws RemoteException {
            Log.i(TAG, "getExtraParam in");
            Log.i(TAG, "getExtraParam key: " + key);
            SoterExtraParam extraParam = new SoterExtraParam();
            try {
                SoterTZService soter = SoterTZService.get();
                if (soter != null && soter.isAvailable()) {
                    Log.i(TAG, "start getExtraParam!");
                    if (isFod()) {
                        if ("fingerprint_type".equals(key)) {
                            extraParam.type = 2;
                        } else if ("fingerprint_hardware_position".equals(key)) {
                            String sensorLoc =
                                    SystemProperties.get(
                                            "persist.vendor.fingerprint.sensor_location", "");
                            if (!sensorLoc.isEmpty()) {
                                try {
                                    String firstEntry = sensorLoc.split(",")[0];
                                    String[] dim = firstEntry.split("\\|");
                                    if (dim.length >= 3) {
                                        int cx = Integer.parseInt(dim[0]);
                                        int cy = Integer.parseInt(dim[1]);
                                        int r = Integer.parseInt(dim[2]);
                                        int startX = cx - r;
                                        int startY = cy - r;
                                        int width = 2 * r;
                                        int height = 2 * r;

                                        extraParam.fp_hw_position[0] = startX;
                                        extraParam.fp_hw_position[1] = startY;
                                        extraParam.fp_hw_position[2] = width;
                                        extraParam.fp_hw_position[3] = height;
                                        Log.i(
                                                TAG,
                                                "fp_hw_position: "
                                                        + startX
                                                        + ","
                                                        + startY
                                                        + ","
                                                        + width
                                                        + ","
                                                        + height);
                                    }
                                } catch (Exception e) {
                                    Log.w(TAG, "Failed to parse sensor_location: " + sensorLoc, e);
                                }
                            }
                        }
                    }
                } else {
                    Log.e(TAG, "soterService is null or unavailable");
                }
            } catch (Exception e) {
                Log.e(TAG, "exception" + e.toString());
            }
            return extraParam;
        }

        @Override
        public String aidlAuth() throws RemoteException {
            return null;
        }
    }
}
