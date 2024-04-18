/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Devices.h"

#define LOG_TAG "Devices"

#include <android-base/logging.h>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

static const std::string kBacklightDevices[] = {
        "backlight",
        "panel0-backlight",
};

static std::vector<BacklightDevice> getBacklightDevices() {
    std::vector<BacklightDevice> devices;

    for (const auto& device : kBacklightDevices) {
        auto backlight = BacklightDevice(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found backlight device " << device;
            devices.push_back(backlight);
        }
    }

    return devices;
}

static const std::string kLedBacklightDevices[] = {
        "lcd-backlight",
};

static std::vector<LedDevice> getBacklightLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kLedBacklightDevices) {
        auto backlight = LedDevice(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found LED backlight device " << device;
            devices.push_back(backlight);
        }
    }

    return devices;
}

static const std::string kButtonLedDevices[] = {
        "button-backlight",
        "button-backlight1",
};

static std::vector<LedDevice> getButtonLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kButtonLedDevices) {
        auto button = LedDevice(device);
        if (button.exists()) {
            LOG(INFO) << "Found button device " << device;
            devices.emplace_back(button);
        }
    }

    return devices;
}

static RgbLedDevice getRgbLedDevice() {
    auto red = LedDevice("red");
    auto green = LedDevice("green");
    auto blue = LedDevice("blue");

    return RgbLedDevice(red, green, blue);
}

static LedDevice getWhiteLedDevice() {
    return LedDevice("white");
}

Devices::Devices()
    : mBacklightDevices(getBacklightDevices()),
      mBacklightLedDevices(getBacklightLedDevices()),
      mButtonLedDevices(getButtonLedDevices()),
      mRgbLedDevice(getRgbLedDevice()),
      mWhiteLed(getWhiteLedDevice()) {
    if (!hasBacklightDevices()) {
        LOG(INFO) << "No backlight devices found";
    }

    if (!hasButtonDevices()) {
        LOG(INFO) << "No button devices found";
    }

    if (!hasNotificationDevices()) {
        LOG(INFO) << "No notification devices found";
    }
}

bool Devices::hasBacklightDevices() {
    return !mBacklightDevices.empty() || !mBacklightLedDevices.empty();
}

bool Devices::hasButtonDevices() {
    return !mButtonLedDevices.empty();
}

bool Devices::hasNotificationDevices() {
    return mRgbLedDevice.exists() || mWhiteLed.exists();
}

void Devices::setBacklightColor(rgb color) {
    for (auto& device : mBacklightDevices) {
        device.setBrightness(color.toBrightness());
    }
    for (auto& device : mBacklightLedDevices) {
        device.setBrightness(color.toBrightness());
    }
}

void Devices::setButtonsColor(rgb color) {
    for (auto& device : mButtonLedDevices) {
        device.setBrightness(color.toBrightness());
    }
}

void Devices::setNotificationColor(rgb color, LightMode mode) {
    bool rc = true;

    switch (mode) {
        case LightMode::BREATH:
            if (mWhiteLed.exists()) {
                rc &= mWhiteLed.setBrightness(color.toBrightness(), LightMode::BREATH);
            } else {
                rc &= mRgbLedDevice.setBrightness(color, LightMode::BREATH);
            }

            if (rc) {
                break;
            }

            LOG(ERROR) << "Breath not supported, falling back to static mode";
            // Fallback to static mode if breath is not supported
            rc = true;

            FALLTHROUGH_INTENDED;
        case LightMode::STATIC:
            if (mWhiteLed.exists()) {
                rc &= mWhiteLed.setBrightness(color.toBrightness(), LightMode::STATIC);
            } else {
                rc &= mRgbLedDevice.setBrightness(color, LightMode::STATIC);
            }

            if (!rc) {
                LOG(ERROR) << "Failed to set color";
            }

            break;
        default:
            LOG(ERROR) << "Unsupported light mode " << mode;
            break;
    }
}

void Devices::dump(int fd) {
    dprintf(fd, "  Backlight devices:\n");
    for (const auto& device : mBacklightDevices) {
        dprintf(fd, "    Name: %s", device.getName().c_str());
        dprintf(fd, ", exists: %d\n", device.exists());
    }

    dprintf(fd, "  Backlight LED devices:\n");
    for (const auto& device : mBacklightLedDevices) {
        dprintf(fd, "    Name: %s", device.getName().c_str());
        dprintf(fd, ", exists: %d\n", device.exists());
    }

    dprintf(fd, "  Button LED devices:\n");
    for (const auto& device : mButtonLedDevices) {
        dprintf(fd, "    Name: %s", device.getName().c_str());
        dprintf(fd, ", exists: %d\n", device.exists());
    }

    dprintf(fd, "  RGB LED device:\n");
    dprintf(fd, "    Exists: %d\n", mRgbLedDevice.exists());

    dprintf(fd, "  White LED device:\n");
    dprintf(fd, "    Name: %s", mWhiteLed.getName().c_str());
    dprintf(fd, ", exists: %d\n", mWhiteLed.exists());

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
