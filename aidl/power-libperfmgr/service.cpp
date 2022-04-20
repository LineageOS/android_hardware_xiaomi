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

#define LOG_TAG "powerhal-libperfmgr"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android/binder_ibinder_platform.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <perfmgr/HintManager.h>

#include <thread>

#include "Power.h"
#include "PowerExt.h"
#include "PowerSessionManager.h"

using aidl::google::hardware::power::impl::pixel::Power;
using aidl::google::hardware::power::impl::pixel::PowerExt;
using aidl::google::hardware::power::impl::pixel::PowerHintMonitor;
using aidl::google::hardware::power::impl::pixel::PowerSessionManager;
using ::android::perfmgr::HintManager;

constexpr std::string_view kPowerHalInitProp("vendor.powerhal.init");

int main() {
    // Parse config but do not start the looper
    std::shared_ptr<HintManager> hm = HintManager::GetInstance();
    if (!hm) {
        LOG(FATAL) << "HintManager Init failed";
    }

    // single thread
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    // core service
    std::shared_ptr<Power> pw = ndk::SharedRefBase::make<Power>();
    ndk::SpAIBinder pwBinder = pw->asBinder();
    AIBinder_setMinSchedulerPolicy(pwBinder.get(), SCHED_NORMAL, -20);

    // extension service
    std::shared_ptr<PowerExt> pwExt = ndk::SharedRefBase::make<PowerExt>();
    auto pwExtBinder = pwExt->asBinder();
    AIBinder_setMinSchedulerPolicy(pwExtBinder.get(), SCHED_NORMAL, -20);

    // attach the extension to the same binder we will be registering
    CHECK(STATUS_OK == AIBinder_setExtension(pwBinder.get(), pwExt->asBinder().get()));

    const std::string instance = std::string() + Power::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(pw->asBinder().get(), instance.c_str());
    CHECK(status == STATUS_OK);
    LOG(INFO) << "Xiaomi Power HAL AIDL Service with Extension is started.";

    if (HintManager::GetInstance()->GetAdpfProfile()) {
        PowerHintMonitor::getInstance()->start();
    }

    std::thread initThread([&]() {
        ::android::base::WaitForProperty(kPowerHalInitProp.data(), "1");
        HintManager::GetInstance()->Start();
    });
    initThread.detach();

    ABinderProcess_joinThreadPool();

    // should not reach
    LOG(ERROR) << "Xiaomi Power HAL AIDL Service with Extension just died.";
    return EXIT_FAILURE;
}
