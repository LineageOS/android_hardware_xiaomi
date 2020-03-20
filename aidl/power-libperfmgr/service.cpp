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

#define LOG_TAG "android.hardware.power-service.xiaomi-libperfmgr"

#include <thread>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include "Power.h"
#include "PowerExt.h"

using aidl::android::hardware::pixel::extension::power::impl::PowerExt;
using aidl::android::hardware::power::impl::pixel::Power;
using ::android::perfmgr::HintManager;

constexpr char kPowerHalConfigPath[] = "/vendor/etc/powerhint.json";
constexpr char kPowerHalInitProp[] = "vendor.powerhal.init";

int main() {
    LOG(INFO) << "Xiaomi Power HAL AIDL Service is starting.";

    // Parse config but do not start the looper
    std::shared_ptr<HintManager> hm = HintManager::GetFromJSON(kPowerHalConfigPath, false);
    if (!hm) {
        LOG(FATAL) << "Invalid config: " << kPowerHalConfigPath;
    }

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    // power hal core service
    std::shared_ptr<Power> pw = ndk::SharedRefBase::make<Power>(hm);
    ndk::SpAIBinder pwBinder = pw->asBinder();

    // making the extension service
    std::shared_ptr<PowerExt> pwExt = ndk::SharedRefBase::make<PowerExt>(hm);

    // need to attach the extension to the same binder we will be registering
    CHECK(STATUS_OK == AIBinder_setExtension(pwBinder.get(), pwExt->asBinder().get()));

    const std::string instance = std::string() + Power::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(pw->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);
    LOG(INFO) << "Xiaomi Power HAL AIDL Service with Extension is started.";

    std::thread initThread([&]() {
        ::android::base::WaitForProperty(kPowerHalInitProp, "1");
        hm->Start();
        pw->setReady();
        pwExt->setReady();
    });
    initThread.detach();

    ABinderProcess_joinThreadPool();
    LOG(ERROR) << "Xiaomi Power HAL AIDL Service died.";
    return EXIT_FAILURE;  // should not reach
}
