/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

int remote_open_input(void);
int remote_inject_key(int fd, uint8_t key);
void remote_close_input(int fd);
