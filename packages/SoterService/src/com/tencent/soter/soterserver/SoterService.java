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

import vendor.qti.hardware.soter.V1_0.ISoter;

import java.lang.ref.WeakReference;
import java.util.List;

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

    private static byte[] listToByteArray(List<Byte> list) {
        if (list == null || list.isEmpty()) {
            return null;
        }
        byte[] arr = new byte[list.size()];
        for (int i = 0; i < list.size(); i++) {
            arr[i] = list.get(i);
        }
        return arr;
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
                        ISoter soter = ISoter.getService();
                        if (soter != null) {
                            Log.i(TAG, "start removeAllAuthKey");
                            soter.removeAllUidKey(uid);
                        } else {
                            Log.e(TAG, "soterService is null");
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
            Log.i(TAG, "generateAppSecureKey in");
            int errcode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start generateAppSecureKey");
                    errcode = soter.generateAskKeyPair(Binder.getCallingUid());
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "generateAppSecureKey errcode:" + errcode);
            return errcode;
        }

        @Override
        public SoterExportResult getAppSecureKey(int uid) throws RemoteException {
            Log.i(TAG, "getAppSecureKeyImp in");
            SoterExportResult result = new SoterExportResult();
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start exportAskPublicKey");
                    soter.exportAskPublicKey(
                            Binder.getCallingUid(),
                            (resCode, data, dataLength) -> {
                                result.resultCode = resCode;
                                result.exportData = listToByteArray(data);
                                result.exportDataLength = dataLength;
                            });
                }
            } catch (Exception e) {
                result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
                Log.e(TAG, "exception" + e.toString());
            }
            return result;
        }

        @Override
        public boolean hasAskAlready(int uid) throws RemoteException {
            Log.v(TAG, "hasAskAlready called");
            int errcode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start hasAskAlready");
                    errcode = soter.hasAskAlready(Binder.getCallingUid());
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            return errcode == SoterErrorCode.SOTER_ERROR_OK;
        }

        @Override
        public int generateAuthKey(int uid, String kname) throws RemoteException {
            Log.i(TAG, "generateAuthKey in");
            if (kname == null || kname.length() == 0) {
                return SoterErrorCode.SOTER_ERROR_KNAME_NULL;
            }
            int errcode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start generateAuthKey");
                    errcode = soter.generateAuthKeyPair(Binder.getCallingUid(), kname);
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "generateAuthKey errcode:" + errcode);
            return errcode;
        }

        @Override
        public int removeAuthKey(int uid, String kname) throws RemoteException {
            Log.i(TAG, "removeAuthKey in");
            if (kname == null || kname.length() == 0) {
                return SoterErrorCode.SOTER_ERROR_KNAME_NULL;
            }
            int errcode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start removeAuthKey");
                    errcode = soter.removeAuthKey(Binder.getCallingUid(), kname);
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "removeAuthKeyImp errcode:" + errcode);
            return errcode;
        }

        @Override
        public SoterExportResult getAuthKey(int uid, String kname) throws RemoteException {
            Log.i(TAG, "getAuthKey in");
            SoterExportResult result = new SoterExportResult();
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            if (kname == null || kname.length() == 0) {
                result.resultCode = SoterErrorCode.SOTER_ERROR_KNAME_NULL;
                return result;
            }
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start exportAuthKeyPublicKey");
                    soter.exportAuthKeyPublicKey(
                            Binder.getCallingUid(),
                            kname,
                            (resCode, data, dataLength) -> {
                                result.resultCode = resCode;
                                result.exportData = listToByteArray(data);
                                result.exportDataLength = dataLength;
                            });
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "exportAuthKeyPublicKey errcode:" + result.resultCode);
            return result;
        }

        @Override
        public int removeAllAuthKey(int uid) throws RemoteException {
            Log.i(TAG, "removeAllAuthKey in");
            int errcode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start removeAllAuthKey");
                    errcode = soter.removeAllUidKey(Binder.getCallingUid());
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "removeAllAuthKeyImp errcode:" + errcode);
            return errcode;
        }

        @Override
        public boolean hasAuthKey(int uid, String kname) throws RemoteException {
            Log.i(TAG, "hasAuthKey in");
            if (kname == null || kname.length() == 0) {
                Log.d(TAG, "kname is null");
                return false;
            }
            int errcode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start hasAuthKeyImp");
                    errcode = soter.hasAuthKey(Binder.getCallingUid(), kname);
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            return errcode == SoterErrorCode.SOTER_ERROR_OK;
        }

        @Override
        public SoterSessionResult initSigh(int uid, String kname, String challenge)
                throws RemoteException {
            Log.i(TAG, "initSigh in");
            SoterSessionResult result = new SoterSessionResult();
            if (kname == null || kname.length() == 0) {
                result.resultCode = SoterErrorCode.SOTER_ERROR_KNAME_NULL;
                return result;
            }
            if (challenge == null || challenge.length() == 0) {
                result.resultCode = SoterErrorCode.SOTER_ERROR_CHALLENGE_NULL;
                return result;
            }
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start initSign");
                    soter.initSign(
                            Binder.getCallingUid(),
                            kname,
                            challenge,
                            (resCode, session) -> {
                                result.resultCode = resCode;
                                result.session = session;
                            });
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "initSign errcode:" + result.resultCode);
            return result;
        }

        @Override
        public SoterSignResult finishSign(long signSession) throws RemoteException {
            Log.i(TAG, "finishSignImp in");
            SoterSignResult result = new SoterSignResult();
            if (signSession == 0) {
                result.resultCode = SoterErrorCode.SOTER_ERROR_OPERATEID_NULL;
            }
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start initSign");
                    soter.finishSign(
                            signSession,
                            (resCode, data, dataLength) -> {
                                result.resultCode = resCode;
                                result.exportData = listToByteArray(data);
                                result.exportDataLength = dataLength;
                            });
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "finishSignImp errcode:" + result.resultCode);
            return result;
        }

        @Override
        public SoterDeviceResult getDeviceId() throws RemoteException {
            Log.i(TAG, "getDeviceIdImp in");
            SoterDeviceResult result = new SoterDeviceResult();
            try {
                ISoter soter = ISoter.getService();
                if (soter != null) {
                    Log.i(TAG, "start getDeviceId");
                    soter.getDeviceId(
                            (resCode, data, dataLength) -> {
                                result.resultCode = resCode;
                                result.exportData = listToByteArray(data);
                                result.exportDataLength = dataLength;
                            });
                } else {
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString());
            }
            Log.i(TAG, "getDeviceId errcode:" + result.resultCode);
            return result;
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
                ISoter soter = ISoter.getService();
                if (soter != null) {
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
                    Log.e(TAG, "soterService is null");
                }
            } catch (Exception e) {
                Log.e(TAG, "exception" + e.toString());
            }
            return extraParam;
        }
    }
}
