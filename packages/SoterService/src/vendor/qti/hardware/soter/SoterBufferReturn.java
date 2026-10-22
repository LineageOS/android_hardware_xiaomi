/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.qti.hardware.soter;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterBufferReturn implements Parcelable {
    public byte[] data;
    public int dataLength;

    public static final Creator<SoterBufferReturn> CREATOR =
            new Creator<SoterBufferReturn>() {
                @Override
                public SoterBufferReturn createFromParcel(Parcel in) {
                    SoterBufferReturn ret = new SoterBufferReturn();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterBufferReturn[] newArray(int size) {
                    return new SoterBufferReturn[size];
                }
            };

    public SoterBufferReturn() {}

    @Override
    public int describeContents() {
        return 0;
    }

    public final void readFromParcel(Parcel parcel) {
        int pos = parcel.dataPosition();
        int len = parcel.readInt();
        if (len < 0) return;
        try {
            this.data = parcel.createByteArray();
            if (parcel.dataPosition() - pos >= len) {
                parcel.setDataPosition(pos + len);
                return;
            }
            this.dataLength = parcel.readInt();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeByteArray(this.data);
        parcel.writeInt(this.dataLength);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
