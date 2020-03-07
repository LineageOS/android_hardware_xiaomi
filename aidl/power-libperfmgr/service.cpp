/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.power-service.pixel-libperfmgr"

#include "Power.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::android::hardware::power::impl::pixel::Power;

int main() {
    LOG(INFO) << "Power HAL AIDL Service for Pixel is starting.";
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    std::shared_ptr<Power> pw = ndk::SharedRefBase::make<Power>();

    const std::string instance = std::string() + Power::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(pw->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);
    LOG(INFO) << "Power HAL AIDL Service for Pixel is started.";

    ABinderProcess_joinThreadPool();
    LOG(ERROR) << "Power HAL AIDL Service for Pixel died.";
    return EXIT_FAILURE;  // should not reach
}
