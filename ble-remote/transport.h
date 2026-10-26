/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <termios.h>

struct remote_transport {
    const char* name;
    uint16_t manufacturer;
    bool (*probe)(void);
    int (*open)(void);
    void (*close)(int fd);
};

extern const struct remote_transport mtk_transport;
extern const struct remote_transport bcm_transport;

extern volatile sig_atomic_t stopping;
int64_t now_ms(void);
int write_all(int fd, const void* buffer, size_t size);
