//
// Copyright (C) 2026 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

package vendor.qti.hardware.soter;

import vendor.qti.hardware.soter.SoterBufferReturn;
import vendor.qti.hardware.soter.SoterInitReturn;

interface ISoter {
    int exportAskPublicKey(int uid, out SoterBufferReturn buffer);
    int exportAttkPublicKey(out SoterBufferReturn buffer);
    int exportAuthKeyPublicKey(int uid, String name, out SoterBufferReturn buffer);
    int finishSign(long session, out SoterBufferReturn buffer);
    int generateAskKeyPair(int uid);
    int generateAttkKeyPair(byte copyNum);
    int generateAuthKeyPair(int uid, String name);
    int getDeviceId(out SoterBufferReturn buffer);
    int hasAskAlready(int uid);
    int hasAuthKey(int uid, String name);
    SoterInitReturn initSign(int uid, String name, String challenge);
    int removeAllUidKey(int uid);
    int removeAuthKey(int uid, String name);
    int verifyAttkKeyPair();
}
