/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.qti.hardware.soter;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterInitReturn implements Parcelable {
    public int status;
    public long session;

    public static final Creator<SoterInitReturn> CREATOR =
            new Creator<SoterInitReturn>() {
                @Override
                public SoterInitReturn createFromParcel(Parcel in) {
                    SoterInitReturn ret = new SoterInitReturn();
                    ret.readFromParcel(in);
                    return ret;
                }

                @Override
                public SoterInitReturn[] newArray(int size) {
                    return new SoterInitReturn[size];
                }
            };

    public SoterInitReturn() {}

    @Override
    public int describeContents() {
        return 0;
    }

    public final void readFromParcel(Parcel parcel) {
        int pos = parcel.dataPosition();
        int len = parcel.readInt();
        if (len < 0) return;
        try {
            this.status = parcel.readInt();
            if (parcel.dataPosition() - pos >= len) {
                parcel.setDataPosition(pos + len);
                return;
            }
            this.session = parcel.readLong();
        } finally {
            parcel.setDataPosition(pos + len);
        }
    }

    @Override
    public final void writeToParcel(Parcel parcel, int flags) {
        int pos = parcel.dataPosition();
        parcel.writeInt(0);
        parcel.writeInt(this.status);
        parcel.writeLong(this.session);
        int end = parcel.dataPosition();
        parcel.setDataPosition(pos);
        parcel.writeInt(end - pos);
        parcel.setDataPosition(end);
    }
}
