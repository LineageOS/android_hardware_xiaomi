/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdio>
#include <utility>

#include "remote_input.h"
#include "transport.h"

namespace xiaomi::ble_remote {

RemoteInput RemoteInput::create() {
    const int fd = open("/dev/uinput", O_WRONLY | O_CLOEXEC);
    if (fd < 0) return {};

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0 || ioctl(fd, UI_SET_KEYBIT, KEY_UP) < 0 ||
        ioctl(fd, UI_SET_KEYBIT, KEY_DOWN) < 0 || ioctl(fd, UI_SET_KEYBIT, KEY_ENTER) < 0) {
        const int error = errno;
        close(fd);
        errno = error;
        return {};
    }

    uinput_user_dev device{};
    snprintf(device.name, sizeof(device.name), "Xiaomi Remote");
    device.id.bustype = BUS_VIRTUAL;
    if (write_all(fd, &device, sizeof(device)) < 0 || ioctl(fd, UI_DEV_CREATE) < 0) {
        const int error = errno;
        close(fd);
        errno = error;
        return {};
    }
    return RemoteInput(fd);
}

RemoteInput::~RemoteInput() {
    reset();
}

RemoteInput::RemoteInput(RemoteInput&& other) noexcept : fd_(std::exchange(other.fd_, -1)) {}

RemoteInput& RemoteInput::operator=(RemoteInput&& other) noexcept {
    if (this != &other) {
        reset();
        fd_ = std::exchange(other.fd_, -1);
    }
    return *this;
}

void RemoteInput::reset() {
    if (fd_ < 0) return;
    ioctl(fd_, UI_DEV_DESTROY);
    close(fd_);
    fd_ = -1;
}

int RemoteInput::inject(uint8_t key) const {
    uint16_t code;
    const char* name;
    switch (key) {
        case 2:
            code = KEY_UP;
            name = "UP";
            break;
        case 4:
            code = KEY_DOWN;
            name = "DOWN";
            break;
        case 8:
            code = KEY_ENTER;
            name = "OK";
            break;
        default:
            errno = EINVAL;
            return -1;
    }

    input_event events[4]{};
    events[0].type = EV_KEY;
    events[0].code = code;
    events[0].value = 1;
    events[1].type = EV_SYN;
    events[1].code = SYN_REPORT;
    events[2].type = EV_KEY;
    events[2].code = code;
    events[3].type = EV_SYN;
    events[3].code = SYN_REPORT;
    fprintf(stderr, "ble-remote: key %s\n", name);
    return write_all(fd_, events, sizeof(events));
}

}  // namespace xiaomi::ble_remote
