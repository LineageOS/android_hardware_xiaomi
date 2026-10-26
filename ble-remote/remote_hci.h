/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace xiaomi::ble_remote {

using HciEvent = std::array<uint8_t, 257>;

int hci_command(int fd, uint16_t opcode, const uint8_t* args, uint8_t size, HciEvent& response);
int hci_read_event(int fd, HciEvent& event, int64_t deadline_ms);
int hci_start_scan(int fd, uint16_t manufacturer);
void hci_stop_scan(int fd);

}  // namespace xiaomi::ble_remote
