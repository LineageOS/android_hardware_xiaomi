/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Accessories.h"

#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android-base/logging.h>

using aidl::vendor::lineage::accessories::Accessories;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Accessories> hal = ::ndk::SharedRefBase::make<Accessories>();

    const std::string instance = std::string(Accessories::descriptor) + "/default";
    binder_status_t status = AServiceManager_addService(hal->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK);

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
