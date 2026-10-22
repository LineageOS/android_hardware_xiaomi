/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.tencent.soter.soterserver;

import android.os.Parcel;
import android.os.Parcelable;

public class SoterExtraParam implements Parcelable {
    public int[] fp_hw_position = new int[4];
    public int type;

    public static final Creator<SoterExtraParam> CREATOR =
            new Creator<SoterExtraParam>() {
                @Override
                public SoterExtraParam createFromParcel(Parcel in) {
                    return new SoterExtraParam(in);
                }

                @Override
                public SoterExtraParam[] newArray(int size) {
                    return new SoterExtraParam[size];
                }
            };

    public SoterExtraParam() {}

    public SoterExtraParam(Parcel in) {
        this.type = in.readInt();
        this.fp_hw_position = in.createIntArray();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeValue(Integer.valueOf(this.type));
        dest.writeIntArray(this.fp_hw_position);
    }
}
