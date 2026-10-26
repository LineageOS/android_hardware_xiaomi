/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "remote_input.h"
#include "transport.h"

int remote_open_input(void) {
    int fd = open("/dev/uinput", O_WRONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    struct uinput_user_dev device = {};
    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) || ioctl(fd, UI_SET_KEYBIT, KEY_UP) ||
        ioctl(fd, UI_SET_KEYBIT, KEY_DOWN) || ioctl(fd, UI_SET_KEYBIT, KEY_ENTER))
        goto fail;
    snprintf(device.name, sizeof(device.name), "Xiaomi recovery remote");
    device.id.bustype = BUS_VIRTUAL;
    if (write_all(fd, &device, sizeof(device)) || ioctl(fd, UI_DEV_CREATE)) goto fail;
    return fd;
fail:
    close(fd);
    return -1;
}

int remote_inject_key(int fd, uint8_t key) {
    uint16_t code = key == 2 ? KEY_UP : key == 4 ? KEY_DOWN : KEY_ENTER;
    struct input_event events[4] = {};
    events[0].type = EV_KEY;
    events[0].code = code;
    events[0].value = 1;
    events[1].type = EV_SYN;
    events[1].code = SYN_REPORT;
    events[2].type = EV_KEY;
    events[2].code = code;
    events[2].value = 0;
    events[3].type = EV_SYN;
    events[3].code = SYN_REPORT;
    fprintf(stderr, "remote: key %s\n", key == 2 ? "UP" : key == 4 ? "DOWN" : "OK");
    return write_all(fd, events, sizeof(events));
}

void remote_close_input(int fd) {
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}
