/*
 * SPDX-FileCopyrightText: 2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package vendor.xiaomi.hardware.displayfeature_aidl;

import vendor.xiaomi.hardware.displayfeature_aidl.IDisplayFeatureCallback;

@VintfStability
interface IDisplayFeature {
    void notifyBrightness(int brightness);
    void registerCallback(int displayId, IDisplayFeatureCallback callback);
    void sendMessage(int messageId, int param, String message);
    void sendPanelCommand(String command);
    void sendPostProcCommand(int commandId, int param);
    void sendRefreshCommand();
    void setFeature(int featureId, int param1, int param2, int param3);
    void setFunction(int functionId, int param1, int param2, int param3);
}
