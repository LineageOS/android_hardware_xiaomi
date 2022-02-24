/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (C) 2022-2023 The LineageOS Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Changes from Qualcomm Innovation Center are provided under the following license:
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#define LOG_TAG "libqtivibratoreffect.xiaomi"

#include <log/log.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "effect.h"

namespace {
const uint32_t kDefaultPlayRateHz = 24000;
std::unordered_map<uint32_t, effect_stream> effect_streams;

std::unique_ptr<effect_stream> readEffectStreamFromFile(uint32_t effect_id) {
    std::string path = "/vendor/etc/vibrator/effect_" + std::to_string(effect_id) + ".bin";

    ALOGV("Reading fifo data for effect %u from path %s", effect_id, path.c_str());

    std::ifstream data(path, std::ios::in | std::ios::binary);
    if (!data.is_open()) {
        ALOGE("Could not open %s while loading fifo data for effect %u", path.c_str(), effect_id);
        return nullptr;
    }

    std::filesystem::path filePath(path);
    std::uintmax_t fileSize = std::filesystem::file_size(filePath);

    std::vector<int8_t> fifo_data(fileSize);
    data.read(reinterpret_cast<char*>(fifo_data.data()), fileSize);

    return std::make_unique<effect_stream>(effect_id, fileSize, kDefaultPlayRateHz,
                                           fifo_data.data());
}
}  // namespace

const struct effect_stream* get_effect_stream(uint32_t effect_id) {
    auto it = effect_streams.find(effect_id);
    if (it == effect_streams.end()) {
        std::unique_ptr<effect_stream> new_effect_stream = readEffectStreamFromFile(effect_id);

        if (new_effect_stream) {
            auto result = effect_streams.emplace(effect_id, *new_effect_stream);
            return &result.first->second;
        }
    } else {
        return &it->second;
    }

    return nullptr;
}
