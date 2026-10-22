/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.soterservice;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterSignResult implements Parcelable {
    public int soterErrorCode;
    public byte[] signature_buf;
    public int signature_len;

    public static final Creator<SoterSignResult> CREATOR =
            new Creator<SoterSignResult>() {
                @Override
                public SoterSignResult createFromParcel(Parcel in) {
                    SoterSignResult ret = new SoterSignResult();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterSignResult[] newArray(int size) {
                    return new SoterSignResult[size];
                }
            };

    public SoterSignResult() {}

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
            this.signature_buf = parcel.createByteArray();
            if (parcel.dataPosition() - pos >= len) return;
            this.signature_len = parcel.readInt();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.soterErrorCode);
        parcel.writeByteArray(this.signature_buf);
        parcel.writeInt(this.signature_len);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
