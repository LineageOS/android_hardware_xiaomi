/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aidlbgservice;

import vendor.xiaomi.hardware.aidlbgservice.Image;

@VintfStability
parcelable CallbackData {
    int cameraId = 0;
    int type = 0;
    int frameNumber = 0;
    int sessionId = 0;
    long timeStampUs = 0;
    String metadata;
    Image[] images;
    boolean reserve = false;
}
