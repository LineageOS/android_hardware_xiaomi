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
        BacklightDevice backlight(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found backlight device: " << backlight.getName();
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
        LedDevice backlight(device);
        if (backlight.exists()) {
            LOG(INFO) << "Found backlight LED device: " << backlight.getName();
            devices.push_back(backlight);
        }
    }

    return devices;
}

static const std::string kButtonLedDevices[] = {
        "button-backlight",
        "button-backlight1",
        "button-backlight2",
};

static std::vector<LedDevice> getButtonLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kButtonLedDevices) {
        LedDevice button(device);
        if (button.exists()) {
            LOG(INFO) << "Found button LED device: " << button.getName();
            devices.emplace_back(button);
        }
    }

    return devices;
}

static const std::string kRgbLedDevices[][3] = {
        {"red", "green", "blue"},
};

static std::vector<RgbLedDevice> getNotificationRgbLedDevices() {
    std::vector<RgbLedDevice> devices;

    for (const auto& device : kRgbLedDevices) {
        LedDevice red(device[0]);
        LedDevice green(device[1]);
        LedDevice blue(device[2]);

        RgbLedDevice rgbLedDevice(red, green, blue);
        if (rgbLedDevice.exists()) {
            LOG(INFO) << "Found notification RGB LED device: " << red.getName() << ", "
                      << green.getName() << ", " << blue.getName();
            devices.emplace_back(red, green, blue);
        }
    }

    return devices;
}

static const std::string kNotificationLedDevices[] = {
        "left",
        "white",
};

static std::vector<LedDevice> getNotificationLedDevices() {
    std::vector<LedDevice> devices;

    for (const auto& device : kNotificationLedDevices) {
        LedDevice notification(device);
        if (notification.exists()) {
            LOG(INFO) << "Found notification LED device: " << notification.getName();
            devices.emplace_back(notification);
        }
    }

    return devices;
}

Devices::Devices()
    : mBacklightDevices(getBacklightDevices()),
      mBacklightLedDevices(getBacklightLedDevices()),
      mButtonLedDevices(getButtonLedDevices()),
      mNotificationRgbLedDevices(getNotificationRgbLedDevices()),
      mNotificationLedDevices(getNotificationLedDevices()) {
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

bool Devices::hasBacklightDevices() const {
    return !mBacklightDevices.empty() || !mBacklightLedDevices.empty();
}

bool Devices::hasButtonDevices() const {
    return !mButtonLedDevices.empty();
}

bool Devices::hasNotificationDevices() const {
    return !mNotificationRgbLedDevices.empty() || !mNotificationLedDevices.empty();
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

void Devices::setNotificationColor(rgb color, LightMode mode, uint32_t flashOnMs,
                                   uint32_t flashOffMs) {
    for (auto& device : mNotificationRgbLedDevices) {
        device.setBrightness(color, mode, flashOnMs, flashOffMs);
    }

    for (auto& device : mNotificationLedDevices) {
        device.setBrightness(color.toBrightness(), mode, flashOnMs, flashOffMs);
    }
}

void Devices::dump(int fd) const {
    dprintf(fd, "Backlight devices:\n");
    for (const auto& device : mBacklightDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }
    dprintf(fd, "\n");

    dprintf(fd, "Backlight LED devices:\n");
    for (const auto& device : mBacklightLedDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }
    dprintf(fd, "\n");

    dprintf(fd, "Button LED devices:\n");
    for (const auto& device : mButtonLedDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }
    dprintf(fd, "\n");

    dprintf(fd, "Notification RGB LED devices:\n");
    for (const auto& device : mNotificationRgbLedDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }
    dprintf(fd, "\n");

    dprintf(fd, "Notification LED devices:\n");
    for (const auto& device : mNotificationLedDevices) {
        dprintf(fd, "- ");
        device.dump(fd);
        dprintf(fd, "\n");
    }

    return;
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
