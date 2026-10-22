/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.tencent.soter.soterserver;

import android.os.SystemProperties;
import android.util.Log;

public abstract class SoterTZService {
    private static final String TAG = "SoterService_TZ";

    private static SoterTZService sInstance;

    public abstract boolean isAvailable();

    public abstract int generateAppSecureKey(int uid);

    public abstract SoterExportResult getAppSecureKey(int uid);

    public abstract boolean hasAskAlready(int uid);

    public abstract int generateAuthKey(int uid, String kname);

    public abstract int removeAuthKey(int uid, String kname);

    public abstract SoterExportResult getAuthKey(int uid, String kname);

    public abstract int removeAllAuthKey(int uid);

    public abstract boolean hasAuthKey(int uid, String kname);

    public abstract SoterSessionResult initSign(int uid, String kname, String challenge);

    public abstract SoterSignResult finishSign(long signSession);

    public abstract SoterDeviceResult getDeviceId();

    public abstract int removeAllUidKey(int uid);

    private static String getSoterSocType() {
        return SystemProperties.get("ro.soc.manufacturer", "");
    }

    private static String getTeeType() {
        int miteeSupport = SystemProperties.getInt("ro.vendor.mtk_mitee_support", -1);
        int iseeSupport = SystemProperties.getInt("ro.vendor.mtk_microtrust_tee_support", -1);
        Log.d(TAG, "mitee_support = " + miteeSupport + " && isee_support = " + iseeSupport);

        if (miteeSupport == 1) {
            return "mitee";
        } else if (iseeSupport == 1) {
            return "isee";
        }
        return "mitee";
    }

    public static synchronized SoterTZService get() {
        if (sInstance != null && sInstance.isAvailable()) {
            return sInstance;
        }

        String socType = getSoterSocType();
        String teeType = getTeeType();
        Log.d(TAG, "SoterSocType: " + socType + ", SoterTeeType: " + teeType);

        if ("Mediatek".equalsIgnoreCase(socType) || "XRing".equalsIgnoreCase(socType)) {
            if ("isee".equalsIgnoreCase(teeType)) {
                try {
                    SoterTZService iseeAidl = new SoterServiceIseeAidl();
                    if (iseeAidl.isAvailable()) {
                        Log.i(TAG, "Using Microtrust ISEE AIDL Soter HAL");
                        sInstance = iseeAidl;
                        return sInstance;
                    }
                } catch (Throwable t) {
                    Log.d(TAG, "Microtrust ISEE AIDL check failed: " + t);
                }
                try {
                    SoterTZService iseeHidl = new SoterServiceIseeHidl();
                    if (iseeHidl.isAvailable()) {
                        Log.i(TAG, "Using Microtrust ISEE HIDL Soter HAL");
                        sInstance = iseeHidl;
                        return sInstance;
                    }
                } catch (Throwable t) {
                    Log.d(TAG, "Microtrust ISEE HIDL check failed: " + t);
                }
            } else {
                try {
                    SoterTZService miteeAidl = new SoterServiceMiteeAidl();
                    if (miteeAidl.isAvailable()) {
                        Log.i(TAG, "Using Xiaomi MiTEE AIDL Soter HAL");
                        sInstance = miteeAidl;
                        return sInstance;
                    }
                } catch (Throwable t) {
                    Log.d(TAG, "Xiaomi MiTEE AIDL check failed: " + t);
                }
                try {
                    SoterTZService miteeHidl = new SoterServiceMiteeHidl();
                    if (miteeHidl.isAvailable()) {
                        Log.i(TAG, "Using Xiaomi MiTEE HIDL Soter HAL");
                        sInstance = miteeHidl;
                        return sInstance;
                    }
                } catch (Throwable t) {
                    Log.d(TAG, "Xiaomi MiTEE HIDL check failed: " + t);
                }
            }
        } else if ("QTI".equalsIgnoreCase(socType)) {
            try {
                SoterTZService qseeAidl = new SoterServiceQseeAidl();
                if (qseeAidl.isAvailable()) {
                    Log.i(TAG, "Using Qualcomm AIDL Soter HAL");
                    sInstance = qseeAidl;
                    return sInstance;
                }
            } catch (Throwable t) {
                Log.d(TAG, "Qualcomm AIDL Soter HAL check failed: " + t);
            }
            try {
                SoterTZService qseeHidl = new SoterServiceQseeHidl();
                if (qseeHidl.isAvailable()) {
                    Log.i(TAG, "Using Qualcomm HIDL Soter HAL");
                    sInstance = qseeHidl;
                    return sInstance;
                }
            } catch (Throwable t) {
                Log.d(TAG, "Qualcomm HIDL Soter HAL check failed: " + t);
            }
        }

        // Generic probe fallback across all supported HAL backends
        try {
            SoterTZService qseeAidl = new SoterServiceQseeAidl();
            if (qseeAidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Qualcomm AIDL Soter HAL");
                sInstance = qseeAidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        try {
            SoterTZService qseeHidl = new SoterServiceQseeHidl();
            if (qseeHidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Qualcomm HIDL Soter HAL");
                sInstance = qseeHidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        try {
            SoterTZService miteeAidl = new SoterServiceMiteeAidl();
            if (miteeAidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Xiaomi MiTEE AIDL Soter HAL");
                sInstance = miteeAidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        try {
            SoterTZService miteeHidl = new SoterServiceMiteeHidl();
            if (miteeHidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Xiaomi MiTEE HIDL Soter HAL");
                sInstance = miteeHidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        try {
            SoterTZService iseeAidl = new SoterServiceIseeAidl();
            if (iseeAidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Microtrust ISEE AIDL Soter HAL");
                sInstance = iseeAidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        try {
            SoterTZService iseeHidl = new SoterServiceIseeHidl();
            if (iseeHidl.isAvailable()) {
                Log.i(TAG, "Probe: Using Microtrust ISEE HIDL Soter HAL");
                sInstance = iseeHidl;
                return sInstance;
            }
        } catch (Throwable ignored) {
        }

        Log.e(TAG, "No supported Soter HAL daemon found!");
        sInstance = new SoterServiceQseeHidl();
        return sInstance;
    }
}
