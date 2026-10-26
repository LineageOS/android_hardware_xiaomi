/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

int hci_command(int fd, uint16_t opcode, const uint8_t* args, uint8_t len, uint8_t response[257]);
int hci_read_event(int fd, uint8_t event[257], int64_t deadline);
int hci_start_scan(int fd, uint16_t manufacturer);
void hci_stop_scan(int fd);
