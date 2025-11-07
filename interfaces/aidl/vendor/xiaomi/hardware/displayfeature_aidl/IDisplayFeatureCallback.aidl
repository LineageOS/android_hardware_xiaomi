/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.displayfeature_aidl;

@VintfStability
interface IDisplayFeatureCallback {
    void displayfeatureInfoChanged(
        int caseId,
        int value,
        float red,
        float green,
        float blue
    );
}
