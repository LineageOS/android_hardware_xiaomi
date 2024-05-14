/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MiPad5Keyboard.h"

#define LOG_TAG "MiPad5Keyboard"

#include <android-base/logging.h>
#include "MiPad5KeyboardCommands.h"
#include "../Utils.h"

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
      mPendingUpdate(false),
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
        LOG(ERROR) << "libusb context is not initialized";
        return;
    }

    // Get the device list
    libusb_device **devices = nullptr;
    ssize_t device_count = libusb_get_device_list(mLibUsbContext, &devices);
    if (device_count < 0) {
        LOG(ERROR) << "Failed to get device list: " << libusb_error_name(device_count);
        return;
    }

    int rc;

    // Get the device
    libusb_device *device = nullptr;
    for (ssize_t i = 0; i < device_count; i++) {
        libusb_device *dev = devices[i];
        struct libusb_device_descriptor desc;
        rc = libusb_get_device_descriptor(dev, &desc);
        if (rc != LIBUSB_SUCCESS) {
            LOG(ERROR) << "Failed to get device descriptor: " << libusb_error_name(rc);
            continue;
        }

        if (desc.idVendor == kVendorId && desc.idProduct == kProductId) {
            device = dev;
            break;
        }
    }
    if (!device) {
        LOG(ERROR) << "Cannot find the USB device";
        libusb_free_device_list(devices, 1);
        return;
    }

    // Open the device handle
    libusb_device_handle* device_handle = nullptr;
    rc = libusb_open(device, &device_handle);
    if (rc != LIBUSB_SUCCESS) {
        LOG(ERROR) << "Failed to open device: " << libusb_error_name(rc);
        libusb_free_device_list(devices, 1);
        return;
    }

    // Free the list
    libusb_free_device_list(devices, 1);

    // Automatically detach the kernel driver
    rc = libusb_set_auto_detach_kernel_driver(device_handle, 1);
    if (rc != LIBUSB_SUCCESS && rc != LIBUSB_ERROR_NOT_FOUND) {
        LOG(ERROR) << "Failed to detach kernel driver: " << libusb_error_name(rc);
        libusb_close(device_handle);
        return;
    }

    // Claim the interface
    rc = libusb_claim_interface(device_handle, 0);
    if (rc != LIBUSB_SUCCESS) {
        LOG(ERROR) << "Failed to claim interface: " << libusb_error_name(rc);
        libusb_close(device_handle);
        return;
    }

    GetConnectStateResult connectStateResult;

    do {
        int transferred;
        // Send the command to check the connection state
        std::array<uint8_t, 64> sendBuffer = {0};
        memcpy(sendBuffer.data(), kCommandGetConnectState.data(), kCommandGetConnectState.size());
        rc = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_OUT, sendBuffer.data(),
                                  sendBuffer.size(), &transferred, 500);
        if (rc != LIBUSB_SUCCESS) {
            LOG(ERROR) << "Failed to send command: " << libusb_error_name(rc);
            break;
        }

        // Receive data from the device, try two times
        std::array<uint8_t, 64> receiveBuffer = {0};
        for (int i = 0; i < 2; i++) {
            rc = libusb_bulk_transfer(device_handle, LIBUSB_ENDPOINT_IN, receiveBuffer.data(),
                                      receiveBuffer.size(), &transferred, 500);
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
        return;
    }

    mConnected = connectStateResult.connect_state == ConnectState::CONNECTED;

    LOG(INFO) << "Connected: " << mConnected;
    LOG(INFO) << "Connection state: " << (int)connectStateResult.connect_state;

    // Tell the driver the connection status
    if (!writeToFile(kKeyboardConnectedPath, mConnected ? "1" : "0")) {
        LOG(ERROR) << "Failed to write to " << kKeyboardConnectedPath;
    }

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
