/*
 * Copyright (C) 2022 The LineageOS Project
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
import android.util.Log;
import com.tencent.soter.soterserver.ISoterService;
import com.tencent.soter.soterserver.SoterDeviceResult;
import com.tencent.soter.soterserver.SoterExportResult;
import com.tencent.soter.soterserver.SoterSessionResult;
import com.tencent.soter.soterserver.SoterSignResult;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;
import vendor.qti.hardware.soter.V1_0.ISoter;

public class SoterService extends Service {
    private static final String LOG_TAG = SoterService.class.getSimpleName();

    private final IBinder mBinder = new ServiceStub(this);

    private ISoter mSoter = null;

    @Override
    public IBinder onBind(Intent intent) {
        return this.mBinder;
    }

    public static class UserAppUninstallReceiver extends BroadcastReceiver {
        public void onReceive(Context context, Intent intent) {
            if (!intent.getAction().equals("android.intent.action.PACKAGE_FULLY_REMOVED")) {
                return;
            }

            int uid = intent.getIntExtra("android.intent.extra.UID", 0);

            if (intent.getBooleanExtra("android.intent.extra.REPLACING", false)) {
                return;
            }

            try {
                ISoter soter = ISoter.getService();
                if (soter == null) {
                    Log.e(LOG_TAG, "soterService is null");
                    return;
                }

                soter.removeAllUidKey(uid);
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }
        }
    }

    private final class ServiceStub extends ISoterService.Stub {
        ServiceStub(SoterService soterService) {
            new WeakReference(soterService);
        }

        @Override
        public int generateAppSecureKey(int i) throws RemoteException {
            int returnCode = -200;

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return returnCode;
            }

            try {
                returnCode = soter.generateAskKeyPair(Binder.getCallingUid());
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return returnCode;
        }

        @Override
        public SoterExportResult getAppSecureKey(int i) throws RemoteException {
            final SoterExportResult soterExportResult = new SoterExportResult();
            soterExportResult.resultCode = -1000;

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return soterExportResult;
            }

            try {
                soter.exportAskPublicKey(Binder.getCallingUid(), new ISoter.exportAskPublicKeyCallback() {
                    @Override
                    public void onValues(int result_code, ArrayList<Byte> data, int data_len) {
                        soterExportResult.resultCode = result_code;
                        soterExportResult.exportData = SoterService.this.listTobyte(data);
                        soterExportResult.exportDataLength = data_len;
                    }
                });
            } catch (Exception e) {
                soterExportResult.resultCode = -200;
                Log.e(LOG_TAG, "exception" + e.toString());
            }

            return soterExportResult;
        }

        @Override
        public boolean hasAskAlready(int i) throws RemoteException {
            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return false;
            }

            try {
                return soter.hasAskAlready(Binder.getCallingUid()) == 0;
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return false;
        }

        @Override
        public int generateAuthKey(int i, String str) throws RemoteException {
            if (str == null || str.length() == 0) {
                return -202;
            }

            int returnCode = -200;

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return returnCode;
            }

            try {
                returnCode = soter.generateAuthKeyPair(Binder.getCallingUid(), str);
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return returnCode;
        }

        @Override
        public int removeAuthKey(int i, String str) throws RemoteException {
            if (str == null || str.length() == 0) {
                return -202;
            }

            int errcode = -200;

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return errcode;
            }

            try {
                errcode = soter.removeAuthKey(Binder.getCallingUid(), str);
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return errcode;
        }

        @Override
        public SoterExportResult getAuthKey(int i, String str) throws RemoteException {
            final SoterExportResult soterExportResult = new SoterExportResult();
            soterExportResult.resultCode = -1000;

            if (str == null || str.length() == 0) {
                soterExportResult.resultCode = -202;
                return soterExportResult;
            }

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return soterExportResult;
            }

            try {
                soter.exportAuthKeyPublicKey(Binder.getCallingUid(), str,
                        new ISoter.exportAuthKeyPublicKeyCallback() {
                            @Override
                            public void onValues(int result_code, ArrayList<Byte> data, int data_len) {
                                soterExportResult.resultCode = result_code;
                                soterExportResult.exportData = SoterService.this.listTobyte(data);
                                soterExportResult.exportDataLength = data_len;
                            }
                        });
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return soterExportResult;
        }

        @Override
        public int removeAllAuthKey(int i) throws RemoteException {
            int returnCode = -200;

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return returnCode;
            }

            try {
                returnCode = soter.removeAllUidKey(Binder.getCallingUid());
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return returnCode;
        }

        @Override
        public boolean hasAuthKey(int i, String kname) throws RemoteException {
            if (kname == null || kname.length() == 0) {
                Log.d(LOG_TAG, "kname is null");
                return false;
            }

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return false;
            }

            try {
                return soter.hasAuthKey(Binder.getCallingUid(), kname) == 0;
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return false;
        }

        @Override
        public SoterSessionResult initSigh(int i, String str, String str2) throws RemoteException {
            final SoterSessionResult soterSessionResult = new SoterSessionResult();
            if (str == null || str.length() == 0) {
                soterSessionResult.resultCode = -202;
                return soterSessionResult;
            }

            if (str2 == null || str2.length() == 0) {
                soterSessionResult.resultCode = -203;
                return soterSessionResult;
            }

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return soterSessionResult;
            }

            try {
                soter.initSign(Binder.getCallingUid(), str, str2, new ISoter.initSignCallback() {
                    @Override
                    public void onValues(int result_code, long session) {
                        soterSessionResult.session = session;
                        soterSessionResult.resultCode = result_code;
                    }
                });
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return soterSessionResult;
        }

        @Override
        public SoterSignResult finishSign(long session) throws RemoteException {
            final SoterSignResult soterSignResult = new SoterSignResult();
            if (session == 0) {
                soterSignResult.resultCode = -204;
            }

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return soterSignResult;
            }

            try {
                soter.finishSign(session, new ISoter.finishSignCallback() {
                    @Override
                    public void onValues(int result_code, ArrayList<Byte> data, int data_len) {
                        soterSignResult.resultCode = result_code;
                        soterSignResult.exportData = SoterService.this.listTobyte(data);
                        soterSignResult.exportDataLength = data_len;
                    }
                });
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return soterSignResult;
        }

        @Override
        public SoterDeviceResult getDeviceId() throws RemoteException {
            final SoterDeviceResult soterDeviceResult = new SoterDeviceResult();

            ISoter soter = getSoter();
            if (soter == null) {
                Log.e(LOG_TAG, "soterService is null");
                return soterDeviceResult;
            }

            try {
                soter.getDeviceId(new ISoter.getDeviceIdCallback() {
                    @Override
                    public void onValues(int result_code, ArrayList<Byte> data, int data_len) {
                        soterDeviceResult.resultCode = result_code;
                        soterDeviceResult.exportData = SoterService.this.listTobyte(data);
                        soterDeviceResult.exportDataLength = data_len;
                    }
                });
            } catch (Exception e) {
                Log.e(LOG_TAG, e.toString());
            }

            return soterDeviceResult;
        }

        @Override
        public int getVersion() throws RemoteException {
            return 1;
        }
    }

    // Utils

    private ISoter getSoter() {
        if (mSoter == null) {
            try {
                mSoter = ISoter.getService();
            } catch (RemoteException e) {
                // do nothing
            }
        }

        return mSoter;
    }

    private byte[] listTobyte(List<Byte> list) {
        if (list == null || list.size() == 0) {
            return null;
        }

        byte[] bArr = new byte[list.size()];

        for (int i = 0; i < list.size(); i++) {
            bArr[i] = list.get(i).byteValue();
        }

        return bArr;
    }
}
