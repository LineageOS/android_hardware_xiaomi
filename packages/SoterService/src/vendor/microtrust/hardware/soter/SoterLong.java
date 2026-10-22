/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.microtrust.hardware.soter;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterLong implements Parcelable {
    public int errorCode;
    public long session;

    public static final Creator<SoterLong> CREATOR =
            new Creator<SoterLong>() {
                @Override
                public SoterLong createFromParcel(Parcel in) {
                    SoterLong ret = new SoterLong();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterLong[] newArray(int size) {
                    return new SoterLong[size];
                }
            };

    public SoterLong() {}

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
            this.session = parcel.readLong();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.errorCode);
        parcel.writeLong(this.session);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
