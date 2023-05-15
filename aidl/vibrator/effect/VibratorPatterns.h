/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 */

#ifndef  VIBRATOR_PATTERNS_H
#define  VIBRATOR_PATTERNS_H

#include <sys/types.h>

struct effect {
    uint16_t effect_id;
    uint16_t effect_type;
    uint16_t effect_len;
    uint16_t offset;
    uint32_t play_rate;
};

enum period {
    S_PERIOD_T_LRA = 0,
    S_PERIOD_T_LRA_DIV_2,
    S_PERIOD_T_LRA_DIV_4,
    S_PERIOD_T_LRA_DIV_8,
    S_PERIOD_T_LRA_X_2,
    S_PERIOD_T_LRA_X_4,
    S_PERIOD_T_LRA_X_8,
    /* F_8KHZ to F_48KHZ can be specified only for FIFO patterns */
    S_PERIOD_F_8KHZ = 8,
    S_PERIOD_F_16KHZ,
    S_PERIOD_F_24KHZ,
    S_PERIOD_F_32KHZ,
    S_PERIOD_F_44P1KHZ,
    S_PERIOD_F_48KHZ,
};

enum effect_type {
    EFFECT_TYPE_PATTERN = 1,
    EFFECT_TYPE_FIFO_ENVELOPE,
    EFFECT_TYPE_FIFO_STREAMING,
};

enum offload_status {
    OFFLOAD_SUCCESS = 0,
    OFFLOAD_FAILURE = 1
};

int get_pattern_config(uint8_t **ptr, uint32_t *size);
int get_pattern_data(uint8_t **ptr, uint32_t *size);
void free_pattern_mem(uint8_t *ptr);
#endif
