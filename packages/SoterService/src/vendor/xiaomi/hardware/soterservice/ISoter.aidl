//
// Copyright (C) 2026 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

package vendor.xiaomi.hardware.soterservice;

import vendor.xiaomi.hardware.soterservice.SoterDeviceResult;
import vendor.xiaomi.hardware.soterservice.SoterExportResult;
import vendor.xiaomi.hardware.soterservice.SoterSessionResult;
import vendor.xiaomi.hardware.soterservice.SoterSignResult;

interface ISoter {
    SoterExportResult[] exportAskPublicKey(int uid);
    SoterExportResult[] exportAttkPublicKey();
    SoterExportResult[] exportAuthKeyPublicKey(int uid, String name);
    SoterSignResult[] finishSign(long session);
    int generateAskKeyPair(int uid);
    int generateAttkKeyPair(int copyNum);
    int generateAuthKeyPair(int uid, String name);
    SoterDeviceResult[] getDeviceId();
    int hasAskAlready(int uid);
    int hasAuthKey(int uid, String name);
    SoterSessionResult[] initSign(int uid, String name, String challenge);
    int removeAllUidKey(int uid);
    int removeAuthKey(int uid, String name);
    int verifyAttkKeyPair();
}
