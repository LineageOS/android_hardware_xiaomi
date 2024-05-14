/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MiPad5Keyboard.h"

#define LOG_TAG "MiPad5Keyboard"

#include <android-base/logging.h>
#include "MiPad5KeyboardCommands.h"
#include "Utils.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static const uint16_t kVendorId = 0x3206;
static const uint16_t kProductId = 0x3FFC;

static const std::string kKeyboardConnectionStatusPath =
        "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_conn_status";
static const std::string kKeyboardConnectedPath =
        "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_connected";
static const std::string kKeyboardEnabledPath =
        "/sys/devices/platform/soc/soc:xiaomi_keyboard/xiaomi_keyboard_enabled";

MiPad5Keyboard::MiPad5Keyboard()
    : mConnectionStatusPoller(kKeyboardConnectionStatusPath, [this]() { onConnectedChanged(); }),
      mEnabledPoller(kKeyboardEnabledPath, [this]() { onEnabledChanged(); }) {
    mAccessoryInfo = {
            .id = "keyboard:internal:1",
            .type = AccessoryType::KEYBOARD,
            .isPersistent = true,
            .displayName = "Xiaomi Keyboard",
            .displayId = "",
            .supportsToggling = true,
            .supportsBatteryQuerying = false,
    };
    mAccessoryStatus = AccessoryStatus::ENABLED;

    LOG(INFO) << "Initializing Mi Pad 5 keyboard";

    int rc = libusb_init_context(&mLibUsbContext, nullptr, 0);
    if (rc != LIBUSB_SUCCESS) {
        LOG(ERROR) << "Failed to initialize libusb: " << libusb_error_name(rc);
        mLibUsbContext = nullptr;
    }

    mConnectionStatusPoller.start();
    mEnabledPoller.start();

    onConnectedChanged();
    onEnabledChanged();
}

MiPad5Keyboard::~MiPad5Keyboard() {
    if (mLibUsbContext) {
        libusb_exit(mLibUsbContext);
        mLibUsbContext = nullptr;
    }
}

::ndk::ScopedAStatus MiPad5Keyboard::setEnabled(bool enabled) {
    if (!writeToFile(kKeyboardEnabledPath, enabled ? "1" : "0")) {
        LOG(ERROR) << "Failed to write to " << kKeyboardEnabledPath;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mEnabled = enabled;

    updateStatus();

    return ::ndk::ScopedAStatus::ok();
}

void MiPad5Keyboard::updateStatus() {
    mAccessoryStatus = !mConnected ? AccessoryStatus::DISCONNECTED
                       : mEnabled  ? AccessoryStatus::ENABLED
                                   : AccessoryStatus::DISABLED;

    onAccessoryStatusUpdated();
}

void MiPad5Keyboard::onConnectedChanged() {
    std::lock_guard lock(mDataMutex);

    if (!mLibUsbContext) {
        LOG(ERROR) << "Libusb context is not initialized";
        return;
    }

    LOG(INFO) << "Checking connection state";

    libusb_device_handle* device_handle =
            libusb_open_device_with_vid_pid(mLibUsbContext, kVendorId, kProductId);
    if (!device_handle) {
        mConnected = false;
        updateStatus();
        return;
    }

    // Claim the interface
    int rc = libusb_claim_interface(device_handle, 0);
    if (rc != LIBUSB_SUCCESS) {
        LOG(ERROR) << "Failed to claim interface: " << libusb_error_name(rc);
        libusb_close(device_handle);
        mConnected = false;
        updateStatus();
        return;
    }

    GetConnectStateResult connectStateResult;

    do {
        // Send the command to check the connection state
        std::array<uint8_t, 64> sendBuffer = {0};
        memcpy(sendBuffer.data(), kCommandGetConnectState.data(), kCommandGetConnectState.size());
        rc = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_OUT, sendBuffer.data(),
                                  sendBuffer.size(), nullptr, 500);
        if (rc != LIBUSB_SUCCESS) {
            LOG(ERROR) << "Failed to send command: " << libusb_error_name(rc);
            mConnected = false;
            updateStatus();
            break;
        }

        // Receive data from the device, try two times
        std::array<uint8_t, 64> receiveBuffer = {0};
        for (int i = 0; i < 2; i++) {
            rc = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_IN, receiveBuffer.data(),
                                      receiveBuffer.size(), nullptr, 500);
            if (rc != LIBUSB_SUCCESS) {
                LOG(ERROR) << "Failed to receive data: " << libusb_error_name(rc);
                continue;
            }

            // Parse the response
            connectStateResult = parseGetConnectStateResponse(receiveBuffer);
            if (connectStateResult.success) {
                break;
            }
        }
    } while (false);

    // Unclaim the interface
    libusb_release_interface(device_handle, 0);

    // Close the device
    libusb_close(device_handle);

    if (!connectStateResult.success) {
        LOG(ERROR) << "Failed to parse response";
        mConnected = false;
        updateStatus();
        return;
    }

    LOG(INFO) << "Connected: " << (int)connectStateResult.connect_state;

    // Tell the driver the connection status
    writeToFile(kKeyboardConnectedPath, mConnected ? "1" : "0");

    mConnected = connectStateResult.connect_state == ConnectState::CONNECTED;

    updateStatus();
}

void MiPad5Keyboard::onEnabledChanged() {
    std::lock_guard lock(mDataMutex);

    if (!readFromFile(kKeyboardEnabledPath, mEnabled)) {
        LOG(ERROR) << "Failed to read from " << kKeyboardEnabledPath;
        return;
    }

    updateStatus();
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
