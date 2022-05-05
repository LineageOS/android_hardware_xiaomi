//
// Copyright (C) 2022 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

package com.tencent.soter.soterserver;

import com.tencent.soter.soterserver.SoterDeviceResult;
import com.tencent.soter.soterserver.SoterExportResult;
import com.tencent.soter.soterserver.SoterSessionResult;
import com.tencent.soter.soterserver.SoterSignResult;

interface ISoterService {
    int generateAppSecureKey(int i);
    SoterExportResult getAppSecureKey(int i);
    boolean hasAskAlready(int i);
    int generateAuthKey(int i, String str);
    int removeAuthKey(int i, String str);
    SoterExportResult getAuthKey(int i, String str);
    int removeAllAuthKey(int i);
    boolean hasAuthKey(int i, String kname);
    SoterSessionResult initSigh(int i, String str, String str2);
    SoterSignResult finishSign(long session);
    SoterDeviceResult getDeviceId();
    int getVersion();
}
