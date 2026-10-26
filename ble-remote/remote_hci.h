/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace xiaomi::ble_remote {

using HciEvent = std::array<uint8_t, 257>;

enum class HciOpcode : uint16_t {
    kReadLocalVersionInformation = 0x1001,
    kReset = 0x0c03,
    kSetEventMask = 0x0c01,
    kLeSetEventMask = 0x2001,
    kLeSetScanParameters = 0x200b,
    kLeSetScanEnable = 0x200c,
};

constexpr uint16_t kBroadcomManufacturerId = 15;
constexpr uint16_t kMediaTekManufacturerId = 70;

constexpr uint8_t kHciCommandPacketType = 0x01;

int hci_command(int fd, HciOpcode opcode, const uint8_t* args, uint8_t size, HciEvent& response);
int hci_read_event(int fd, HciEvent& event, int64_t deadline_ms);
int hci_start_scan(int fd, uint16_t manufacturer);
void hci_stop_scan(int fd);

}  // namespace xiaomi::ble_remote
