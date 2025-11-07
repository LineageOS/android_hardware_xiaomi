/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aidlbgservice;

import vendor.xiaomi.hardware.aidlbgservice.IEventCallback;

@VintfStability
interface IBGService {
    void setCapabilities(String str);
    int setEventCallback(int i, IEventCallback iEventCallback);
}
