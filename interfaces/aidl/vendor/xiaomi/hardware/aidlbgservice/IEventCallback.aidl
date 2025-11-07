/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aidlbgservice;

import vendor.xiaomi.hardware.aidlbgservice.CallbackData;
import vendor.xiaomi.hardware.aidlbgservice.CallbackInfo;

@VintfStability
interface IEventCallback {
    String notifyCallback(in CallbackData callbackData);
    String notifySnapshotAvailability(int available);
    String notifyCallbackInfo(in CallbackInfo info);
    String onCaptureCompleted(String fileName, int frameNumber);
    String onCaptureFailed(String fileName, int frameNumber);
}
