/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <cstdint>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

// Get connect state command

enum class ConnectState {
    OVERCHARGED,
    TRX_CHECK_FAILED,
    PIN_CONNECT_FAILED,
    DISCONNECTED,
    CONNECTED,
    UNKNOWN,
};

struct GetConnectStateResult {
    bool success = false;
    ConnectState connect_state = ConnectState::UNKNOWN;
    uint16_t voltage = 0x0000;
    uint16_t clock_off = 0x0000;     // Commented as E_UART
    uint32_t true_off = 0x00000000;  // Commented as E_PPM
};

extern const std::array<uint8_t, 7> kCommandGetConnectState;

const GetConnectStateResult parseGetConnectStateResponse(const std::array<uint8_t, 64>& response);

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
