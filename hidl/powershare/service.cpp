/*
 * Copyright (C) 2020 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.powershare@1.0-service.xiaomi"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "PowerShare.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

using vendor::lineage::powershare::V1_0::IPowerShare;
using vendor::lineage::powershare::V1_0::implementation::PowerShare;

using android::OK;
using android::status_t;

int main() {
    android::sp<IPowerShare> service = new PowerShare();

    configureRpcThreadpool(1, true);

    status_t status = service->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Cannot register PowerShare HAL service.";
        return 1;
    }

    LOG(INFO) << "PowerShare HAL service ready.";

    joinRpcThreadpool();

    LOG(ERROR) << "PowerShare HAL service failed to join thread pool.";
    return 1;
}
