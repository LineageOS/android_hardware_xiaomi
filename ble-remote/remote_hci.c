/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "remote_hci.h"
#include "transport.h"

static int read_until(int fd, uint8_t* buffer, size_t size, int64_t deadline) {
    while (size && !stopping) {
        int64_t remaining = deadline - now_ms();
        if (remaining <= 0) {
            errno = ETIMEDOUT;
            return -1;
        }
        struct pollfd pfd = {0};
        pfd.fd = fd;
        pfd.events = POLLIN;
        int ready = poll(&pfd, 1, (int)remaining);
        if (ready < 0 && errno == EINTR) continue;
        if (ready == 0) {
            errno = ETIMEDOUT;
            return -1;
        }
        if (ready < 0) return -1;
        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            errno = EIO;
            return -1;
        }
        ssize_t n = read(fd, buffer, size);
        if (n < 0 && (errno == EINTR || errno == EAGAIN)) continue;
        if (n <= 0) {
            if (!n) errno = EIO;
            return -1;
        }
        buffer += n;
        size -= n;
    }
    if (size) {
        errno = EINTR;
        return -1;
    }
    return 0;
}

int hci_read_event(int fd, uint8_t event[257], int64_t deadline) {
    uint8_t type;
    do {
        if (read_until(fd, &type, 1, deadline)) return -1;
    } while (type != 4);
    if (read_until(fd, event, 2, deadline) || read_until(fd, event + 2, event[1], deadline)) {
        if (errno == ETIMEDOUT) errno = EPROTO;
        return -1;
    }
    return event[1] + 2;
}

int hci_command(int fd, uint16_t opcode, const uint8_t* args, uint8_t len, uint8_t response[257]) {
    uint8_t packet[259] = {1, (uint8_t)(opcode & 0xff), (uint8_t)(opcode >> 8), len};
    if (len) memcpy(packet + 4, args, len);
    if (write_all(fd, packet, len + 4)) return -1;
    int64_t deadline = now_ms() + 2000;
    while (!stopping) {
        int n = hci_read_event(fd, response, deadline);
        if (n < 0) break;
        if (n >= 6 && response[0] == 0x0e && response[3] == (opcode & 0xff) &&
            response[4] == (opcode >> 8)) {
            if (response[5] == 0) return n;
            fprintf(stderr, "remote: command %04x status %02x\n", opcode, response[5]);
            errno = EPROTO;
            return -1;
        }
    }
    if (stopping) errno = EINTR;
    fprintf(stderr, "remote: command %04x failed: %s\n", opcode, strerror(errno));
    return -1;
}

int hci_start_scan(int fd, uint16_t manufacturer) {
    uint8_t event[257];
    int n = hci_command(fd, 0x1001, NULL, 0, event);
    if (n < 14) {
        if (n >= 0) errno = EPROTO;
        return -1;
    }
    uint16_t actual = event[10] | ((uint16_t)event[11] << 8);
    fprintf(stderr, "remote: HCI version %u, manufacturer %u\n", event[6], actual);
    if (actual != manufacturer) {
        errno = ENODEV;
        return -1;
    }
    if (hci_command(fd, 0x0c03, NULL, 0, event) < 0) return -1;
    if (manufacturer == 70) {
        const uint8_t event_mask[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f};
        const uint8_t le_mask[] = {0x02, 0, 0, 0, 0, 0, 0, 0};
        if (hci_command(fd, 0x0c01, event_mask, sizeof(event_mask), event) < 0 ||
            hci_command(fd, 0x2001, le_mask, sizeof(le_mask), event) < 0)
            return -1;
    }
    const uint8_t parameters[] = {1, 0x80, 0, 0x36, 0, 0, 0};
    const uint8_t enable[] = {1, 0};
    if (hci_command(fd, 0x200b, parameters, sizeof(parameters), event) < 0 ||
        hci_command(fd, 0x200c, enable, sizeof(enable), event) < 0)
        return -1;
    return 0;
}

void hci_stop_scan(int fd) {
    const uint8_t disable[] = {1, 0x0c, 0x20, 2, 0, 0};
    (void)write_all(fd, disable, sizeof(disable));
}
