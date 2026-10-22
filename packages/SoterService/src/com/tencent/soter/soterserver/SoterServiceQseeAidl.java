/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.tencent.soter.soterserver;

import android.os.IBinder;
import android.os.ServiceManager;
import android.util.Log;

import vendor.qti.hardware.soter.ISoter;
import vendor.qti.hardware.soter.SoterBufferReturn;
import vendor.qti.hardware.soter.SoterInitReturn;

public class SoterServiceQseeAidl extends SoterTZService {
    private static final String TAG = "SoterServiceQseeAidl";
    private static final String SOTER_AIDL_INTERFACE = "vendor.qti.hardware.soter.ISoter/default";

    private ISoter mService;

    private synchronized ISoter getService() {
        if (mService == null) {
            try {
                IBinder binder = ServiceManager.getService(SOTER_AIDL_INTERFACE);
                if (binder != null) {
                    mService = ISoter.Stub.asInterface(binder);
                    if (mService != null) {
                        Log.d(TAG, "SoterService AIDL daemon interface is bound!");
                    } else {
                        Log.e(TAG, "Getting SoterService AIDL daemon interface failed!");
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, "Exception getting AIDL service: " + e);
            }
        }
        return mService;
    }

    @Override
    public boolean isAvailable() {
        try {
            return getService() != null;
        } catch (Exception e) {
            Log.d(TAG, "isAvailable: exception " + e);
            return false;
        }
    }

    @Override
    public int generateAppSecureKey(int uid) {
        Log.i(TAG, "generateAppSecureKey in");
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start generateAppSecureKey");
                errCode = soter.generateAskKeyPair(uid);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            errCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "generateAppSecureKey errcode: " + errCode);
        return errCode;
    }

    @Override
    public SoterExportResult getAppSecureKey(int uid) {
        Log.i(TAG, "getAppSecureKey in");
        SoterExportResult result = new SoterExportResult();
        SoterBufferReturn buffer = new SoterBufferReturn();
        result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start exportAskPublicKey");
                result.resultCode = soter.exportAskPublicKey(uid, buffer);
                result.exportData = buffer.data;
                result.exportDataLength = buffer.dataLength;
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "getAppSecureKey errcode: " + result.resultCode);
        return result;
    }

    @Override
    public boolean hasAskAlready(int uid) {
        Log.v(TAG, "hasAskAlready called");
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.v(TAG, "start hasAskAlready");
                errCode = soter.hasAskAlready(uid);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "hasAskAlready errcode: " + errCode);
        return errCode == SoterErrorCode.SOTER_ERROR_OK;
    }

    @Override
    public int generateAuthKey(int uid, String kname) {
        Log.i(TAG, "generateAuthKey in");
        if (kname == null) {
            Log.e(TAG, "generateAuthKey authKeyName null");
            return SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
        }
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start generateAuthKey");
                errCode = soter.generateAuthKeyPair(uid, kname);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            errCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "generateAuthKey errcode: " + errCode);
        return errCode;
    }

    @Override
    public int removeAuthKey(int uid, String kname) {
        Log.i(TAG, "removeAuthKey in");
        if (kname == null) {
            Log.e(TAG, "removeAuthKey kname null");
            return SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
        }
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start removeAuthKey");
                errCode = soter.removeAuthKey(uid, kname);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            errCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "removeAuthKey errcode: " + errCode);
        return errCode;
    }

    @Override
    public SoterExportResult getAuthKey(int uid, String kname) {
        Log.i(TAG, "getAuthKey in");
        SoterExportResult result = new SoterExportResult();
        result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        if (kname == null) {
            Log.e(TAG, "getAuthKey kname null");
            return result;
        }
        SoterBufferReturn buffer = new SoterBufferReturn();
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start getAuthKey");
                result.resultCode = soter.exportAuthKeyPublicKey(uid, kname, buffer);
                result.exportData = buffer.data;
                result.exportDataLength = buffer.dataLength;
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "getAuthKey errcode: " + result.resultCode);
        return result;
    }

    @Override
    public int removeAllAuthKey(int uid) {
        Log.i(TAG, "removeAllAuthKey in");
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start removeAllAuthKey");
                errCode = soter.removeAllUidKey(uid);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            errCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "removeAllAuthKey errcode: " + errCode);
        return errCode;
    }

    @Override
    public boolean hasAuthKey(int uid, String kname) {
        Log.v(TAG, "hasAuthKey in");
        if (kname == null) {
            Log.e(TAG, "hasAuthKey kname null");
            return false;
        }
        int errCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.v(TAG, "start hasAuthKey");
                errCode = soter.hasAuthKey(uid, kname);
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "hasAuthKey errcode: " + errCode);
        return errCode == SoterErrorCode.SOTER_ERROR_OK;
    }

    @Override
    public SoterSessionResult initSign(int uid, String kname, String challenge) {
        Log.i(TAG, "initSign in");
        SoterSessionResult result = new SoterSessionResult();
        result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        if (kname == null || challenge == null) {
            Log.e(TAG, "initSign kname or challenge null");
            return result;
        }
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start initSign");
                SoterInitReturn ret = soter.initSign(uid, kname, challenge);
                if (ret != null) {
                    result.session = ret.session;
                    result.resultCode = ret.status;
                }
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "initSign errcode: " + result.resultCode);
        return result;
    }

    @Override
    public SoterSignResult finishSign(long signSession) {
        Log.i(TAG, "finishSign in");
        SoterSignResult result = new SoterSignResult();
        result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        SoterBufferReturn buffer = new SoterBufferReturn();
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start finishSign");
                result.resultCode = soter.finishSign(signSession, buffer);
                result.exportData = buffer.data;
                result.exportDataLength = buffer.dataLength;
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "finishSign errcode: " + result.resultCode);
        return result;
    }

    @Override
    public SoterDeviceResult getDeviceId() {
        Log.i(TAG, "getDeviceId in");
        SoterDeviceResult result = new SoterDeviceResult();
        result.resultCode = SoterErrorCode.SOTER_WRAPPERERROR_UNKNOWN;
        SoterBufferReturn buffer = new SoterBufferReturn();
        try {
            ISoter soter = getService();
            if (soter != null) {
                Log.i(TAG, "start getDeviceId");
                result.resultCode = soter.getDeviceId(buffer);
                result.exportData = buffer.data;
                result.exportDataLength = buffer.dataLength;
            } else {
                Log.e(TAG, "soterService is null");
            }
        } catch (Exception e) {
            result.resultCode = SoterErrorCode.SOTER_ERROR_UNKNOWN_ERROR;
            Log.e(TAG, "exception " + e);
        }
        Log.i(TAG, "getDeviceId errcode: " + result.resultCode);
        return result;
    }

    @Override
    public int removeAllUidKey(int uid) {
        return removeAllAuthKey(uid);
    }
}
