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
#include <sys/stat.h>
#include <fstream>
#include <iostream>

#include "effect.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

static const int8_t primitive_0[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const int8_t primitive_1[] = {
    17,  34,  50,  65,  79,  92,  103, 112, 119, 124,
    127, 127, 126, 122, 116, 108, 98,  86,  73,  58,
    42,  26,  9,   -8,  -25, -41, -57, -72, -85, -97,
    -108, -116, -122, -126, -127, -127, -125, -120,
    -113, -104, -93,  -80, -66, -51, -35, -18, -1,
};

static const int8_t primitive_2[] = {
    17,  34,  50,  65,  79,  92,  103, 112, 119, 124,
    127, 127, 126, 122, 116, 108, 98,  86,  73,  58,
    42,  26,  9,   -8,  -25, -41, -57, -72, -85, -97,
    -108, -116, -122, -126, -127, -127, -125, -120,
    -113, -104, -93,  -80, -66, -51, -35, -18, -1,
};

static const struct effect_stream primitives[] = {
    {
        .effect_id = 0,
        .data = primitive_0,
        .length = ARRAY_SIZE(primitive_0),
        .play_rate_hz = 8000,
    },

    {
        .effect_id = 1,
        .data = primitive_1,
        .length = ARRAY_SIZE(primitive_1),
        .play_rate_hz = 8000,
    },

    {
        .effect_id = 2,
        .data = primitive_2,
        .length = ARRAY_SIZE(primitive_2),
        .play_rate_hz = 8000,
    },
};

int parse_custom_data(effect_stream* effect) {
    std::string path = "/vendor/etc/vibrator/effect_" + std::to_string(effect->effect_id) + ".bin";
    std::ifstream data;
    struct stat file_stat;
    int rc;

    ALOGV("Parsing custom fifo data for effect %u from path %s", effect->effect_id, path.c_str());

    rc = stat(path.c_str(), &file_stat);
    if (!rc) {
        effect->length = file_stat.st_size;
    } else {
        ALOGE("Could not open %s while loading fifo data for effect %u", path.c_str(),
              effect->effect_id);
        return rc;
    }

    // 8-bit int array which contains the fifo data, where one slot
    // of the array contains one byte of the fifo data from vendor.
    int8_t* custom_data = new int8_t[effect->length];

    data.open(path.c_str(), std::ios::in | std::ios::binary);
    data.read(reinterpret_cast<char*>(custom_data), effect->length);
    data.close();

    effect->data = custom_data;

    return rc;
}

static effect_stream effect = {
        .effect_id = UINT32_MAX,
        .data = 0,
        .length = 0,
        .play_rate_hz = 24000,
};

const struct effect_stream *get_effect_stream(uint32_t effect_id)
{
    int i;

    if ((effect_id & 0x8000) != 0) {
        effect_id = effect_id & 0x7fff;

        for (i = 0; i < ARRAY_SIZE(primitives); i++) {
            if (effect_id == primitives[i].effect_id)
                return &primitives[i];
        }
    } else {
        if (effect_id == effect.effect_id) {
            return &effect;
        }
        effect.effect_id = effect_id;
        if (!parse_custom_data(&effect)) {
            return &effect;
        }
    }

    return NULL;
}
