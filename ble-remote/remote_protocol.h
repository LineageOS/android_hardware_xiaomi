/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Xiaomi's remote transmits legacy LE advertising data, not HID reports. */
static inline uint8_t remote_key(const uint8_t* data, size_t size) {
    bool remote = false;
    uint8_t key = 0;
    for (size_t pos = 0; pos < size;) {
        size_t len = data[pos];
        if (!len) break;
        if (len >= size - pos) return 0;
        const uint8_t* ad = data + pos + 1;
        if (ad[0] == 0x08 && len == 6 && ad[1] == 'M' && ad[2] == 'I' && ad[3] == ' ' &&
            ad[4] == 'R' && ad[5] == 'C')
            remote = true;
        if (ad[0] == 0xff && len >= 2) key = ad[1];
        pos += len + 1;
    }
    return remote && (key == 2 || key == 4 || key == 8) ? key : 0;
}
