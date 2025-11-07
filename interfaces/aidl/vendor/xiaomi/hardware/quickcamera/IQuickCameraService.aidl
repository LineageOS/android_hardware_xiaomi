/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.quickcamera;

@VintfStability
interface IQuickCameraService {
    void sendEvent(int event);
    void sendEventWithParam(int event, String parameter);
}
