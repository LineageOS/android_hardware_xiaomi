/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.soterservice;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterSessionResult implements Parcelable {
    public int soterErrorCode;
    public long session;
    public int session_len;

    public static final Creator<SoterSessionResult> CREATOR =
            new Creator<SoterSessionResult>() {
                @Override
                public SoterSessionResult createFromParcel(Parcel in) {
                    SoterSessionResult ret = new SoterSessionResult();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterSessionResult[] newArray(int size) {
                    return new SoterSessionResult[size];
                }
            };

    public SoterSessionResult() {}

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
            this.session = parcel.readLong();
            if (parcel.dataPosition() - pos >= len) return;
            this.session_len = parcel.readInt();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.soterErrorCode);
        parcel.writeLong(this.session);
        parcel.writeInt(this.session_len);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
