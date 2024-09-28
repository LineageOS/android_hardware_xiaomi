/*
 * Copyright (C) 2024 The Android Open Source Project
 *               2024 Paranoid Android
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "config/Config.h"

namespace aidl::android::hardware::biometrics::fingerprint {

class FingerprintConfig : public Config {
    Config::Data* getConfigData(int* size) override;
};

}  // namespace aidl::android::hardware::biometrics::fingerprint
