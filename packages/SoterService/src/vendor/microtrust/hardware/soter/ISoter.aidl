//
// Copyright (C) 2026 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

package vendor.microtrust.hardware.soter;

import vendor.microtrust.hardware.soter.SoterByte;
import vendor.microtrust.hardware.soter.SoterLong;

interface ISoter {
    SoterByte exportAskPublicKey(int uid);
    SoterByte exportAttkPublicKey();
    SoterByte exportAuthKeyPublicKey(int uid, String name);
    SoterByte finishSign(long session);
    int generateAskKeyPair(int uid);
    int generateAttkKeyPair(byte copyNum);
    int generateAuthKeyPair(int uid, String name);
    SoterByte getDeviceId();
    int hasAskAlready(int uid);
    int hasAuthKey(int uid, String name);
    SoterLong initSign(int uid, String name, String challenge);
    int removeAllUidKey(int uid);
    int removeAuthKey(int uid, String name);
    int verifyAttkKeyPair();
}
