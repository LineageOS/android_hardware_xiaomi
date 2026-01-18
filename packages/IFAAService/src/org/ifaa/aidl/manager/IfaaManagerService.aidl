//
// Copyright (C) 2022 The LineageOS Project
//
// SPDX-License-Identifier: Apache-2.0
//

package org.ifaa.aidl.manager;

interface IfaaManagerService {
    int getSupportBIOTypes();
    int startBIOManager(int authType);
    String getDeviceModel();
    byte[] processCmd(inout byte[] param);
    int getVersion();
    String getExtInfo(int authType, String keyExtInfo);
    void setExtInfo(int authType, String keyExtInfo, String valExtInfo);
    int getEnabled(int bioType);
    int[] getIDList(int bioType);
}
