/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.microtrust.hardware.soter;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterByte implements Parcelable {
    public int errorCode;
    public byte[] data;
    public int dataLength;

    public static final Creator<SoterByte> CREATOR =
            new Creator<SoterByte>() {
                @Override
                public SoterByte createFromParcel(Parcel in) {
                    SoterByte ret = new SoterByte();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterByte[] newArray(int size) {
                    return new SoterByte[size];
                }
            };

    public SoterByte() {}

    @Override
    public int describeContents() {
        return 0;
    }

    public final void readFromParcel(Parcel parcel) {
        int pos = parcel.dataPosition();
        int len = parcel.readInt();
        if (len < 0) return;
        try {
            this.errorCode = parcel.readInt();
            if (parcel.dataPosition() - pos >= len) return;
            this.data = parcel.createByteArray();
            if (parcel.dataPosition() - pos >= len) return;
            this.dataLength = parcel.readInt();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.errorCode);
        parcel.writeByteArray(this.data);
        parcel.writeInt(this.dataLength);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
