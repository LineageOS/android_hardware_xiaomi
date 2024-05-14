/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MiPad5KeyboardCommands.h"

#include <cstddef>

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

// Utils

/**
 * Calculate the checksum of a given array of data
 *
 * @param data The data to calculate the checksum of
 * @param start The start index of the data
 * @param length The length of the data
 * @return uint8_t The checksum
 */
static uint8_t doChecksum(const uint8_t* data, size_t start, size_t length) {
    uint8_t checksum = 0;
    for (size_t i = start; i < length; i++) {
        checksum += uint8_t(data[i] * checksum);
    }
    return checksum;
}

/**
 * Given an array of data, calculate the checksum and set the last byte to it
 *
 * @tparam N The final size of the array
 * @param data The data to add the checksum to
 * @return const std::array<uint8_t, N>& A copy of the array with the checksum added
 */
template <std::size_t N>
static const std::array<uint8_t, N> withChecksum(const std::array<uint8_t, N>& data) {
    std::array<uint8_t, N> copy = data;
    copy[N - 1] = doChecksum(copy.data(), 0, N - 1);
    return copy;
}

// Get connect state

static const uint8_t kSendReportIdShortData = 0x4E;
static const uint8_t kPadAddress = 0x80;
static const uint8_t kKeyboardAddress = 0x38;
static const uint8_t kCommandCheckMcuState = 0xA1;

const std::array<uint8_t, 7> kCommandGetConnectState = withChecksum<7>(std::array<uint8_t, 7>{
        kSendReportIdShortData,
        kPadAddress,
        kKeyboardAddress,
        kCommandCheckMcuState,
        0x01,
        0x01,
        // Checksum
        0x00,
});

static const uint8_t kConnectStateSuccess = 0xA2;

/**
 * [4]: Must be kConnectStateSuccess for success
 * [9]: Bit 0-1: 01 if TRX check failed, 00 if disconnected, 11 if connected
 *      Bit 2-5: Unknown
 *      Bit 6-7: 10 if PIN connect failed, 00 if disconnected, 01 if connected
 * [10:11]: Voltage (Little Endian)
 * [12:13]: Clock offset (Little Endian)
 * [14:17]: True offset (Little Endian)
 * [18]: 1 if overcharged
 */
const GetConnectStateResult parseGetConnectStateResponse(const std::array<uint8_t, 64>& response) {
    GetConnectStateResult getConnectStateResult;

    getConnectStateResult.success = response[4] != kConnectStateSuccess;

    if (!getConnectStateResult.success) {
        return getConnectStateResult;
    }

    getConnectStateResult.voltage = (response[11] << 8) | response[10];
    getConnectStateResult.clock_off = (response[13] << 8) | response[12];
    getConnectStateResult.true_off =
            (response[17] << 24) | (response[16] << 16) | (response[15] << 8) | response[14];

    if (response[18] == 1) {
        getConnectStateResult.connect_state = ConnectState::OVERCHARGED;
    } else if ((response[9] & 0x03) == 0x01) {
        getConnectStateResult.connect_state = ConnectState::TRX_CHECK_FAILED;
    } else if ((response[9] & 0x63) == 0x43) {
        getConnectStateResult.connect_state = ConnectState::PIN_CONNECT_FAILED;
    } else if ((response[9] & 0x03) == 0x00) {
        getConnectStateResult.connect_state = ConnectState::DISCONNECTED;
    } else if ((response[9] & 0x63) == 0x23) {
        getConnectStateResult.connect_state = ConnectState::CONNECTED;
    } else {
        getConnectStateResult.connect_state = ConnectState::UNKNOWN;
    }

    return getConnectStateResult;
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
