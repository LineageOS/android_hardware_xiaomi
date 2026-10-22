/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.soterservice;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterDeviceResult implements Parcelable {
    public int soterErrorCode;
    public byte[] device_id;
    public int device_id_len;

    public static final Creator<SoterDeviceResult> CREATOR =
            new Creator<SoterDeviceResult>() {
                @Override
                public SoterDeviceResult createFromParcel(Parcel in) {
                    SoterDeviceResult ret = new SoterDeviceResult();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterDeviceResult[] newArray(int size) {
                    return new SoterDeviceResult[size];
                }
            };

    public SoterDeviceResult() {}

    @Override
    public int describeContents() {
        return 0;
    }

    public final void readFromParcel(Parcel parcel) {
        int pos = parcel.dataPosition();
        int len = parcel.readInt();
        if (len < 0) return;
        try {
            this.soterErrorCode = parcel.readInt();
            if (parcel.dataPosition() - pos >= len) return;
            this.device_id = parcel.createByteArray();
            if (parcel.dataPosition() - pos >= len) return;
            this.device_id_len = parcel.readInt();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.soterErrorCode);
        parcel.writeByteArray(this.device_id);
        parcel.writeInt(this.device_id_len);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
