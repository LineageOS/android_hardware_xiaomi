/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.aon;

@VintfStability
interface IAlwaysOnListener {
    void onCallbackListener(int Type, in int[] data);
}
