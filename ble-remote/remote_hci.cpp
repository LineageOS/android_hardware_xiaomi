/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <poll.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include "remote_hci.h"
#include "transport.h"

namespace xiaomi::ble_remote {
namespace {

int read_until(int fd, uint8_t* buffer, size_t size, int64_t deadline_ms) {
    while (size && !stopping) {
        const int64_t remaining = deadline_ms - now_ms();
        if (remaining <= 0) {
            errno = ETIMEDOUT;
            return -1;
        }

        pollfd descriptor{};
        descriptor.fd = fd;
        descriptor.events = POLLIN;
        const int ready = poll(&descriptor, 1, static_cast<int>(remaining));
        if (ready < 0 && errno == EINTR) continue;
        if (ready == 0) {
            errno = ETIMEDOUT;
            return -1;
        }
        if (ready < 0) return -1;
        if (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            errno = EIO;
            return -1;
        }

        const ssize_t count = read(fd, buffer, size);
        if (count < 0 && (errno == EINTR || errno == EAGAIN)) continue;
        if (count <= 0) {
            if (count == 0) errno = EIO;
            return -1;
        }
        buffer += count;
        size -= static_cast<size_t>(count);
    }
    if (size) {
        errno = EINTR;
        return -1;
    }
    return 0;
}

}  // namespace

int hci_read_event(int fd, HciEvent& event, int64_t deadline_ms) {
    uint8_t packet_type;
    do {
        if (read_until(fd, &packet_type, 1, deadline_ms) < 0) return -1;
    } while (packet_type != 4);

    if (read_until(fd, event.data(), 2, deadline_ms) < 0 ||
        read_until(fd, event.data() + 2, event[1], deadline_ms) < 0) {
        if (errno == ETIMEDOUT) errno = EPROTO;
        return -1;
    }
    return event[1] + 2;
}

int hci_command(int fd, uint16_t opcode, const uint8_t* args, uint8_t size,
                HciEvent& response) {
    std::array<uint8_t, 259> packet{};
    packet[0] = 1;
    packet[1] = static_cast<uint8_t>(opcode & 0xff);
    packet[2] = static_cast<uint8_t>(opcode >> 8);
    packet[3] = size;
    if (size) memcpy(packet.data() + 4, args, size);
    if (write_all(fd, packet.data(), size + 4) < 0) return -1;

    const int64_t deadline = now_ms() + 2000;
    while (!stopping) {
        const int length = hci_read_event(fd, response, deadline);
        if (length < 0) break;
        if (length >= 6 && response[0] == 0x0e && response[3] == (opcode & 0xff) &&
            response[4] == (opcode >> 8)) {
            if (response[5] == 0) return length;
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
    HciEvent event{};
    const int length = hci_command(fd, 0x1001, nullptr, 0, event);
    if (length < 14) {
        if (length >= 0) errno = EPROTO;
        return -1;
    }
    const uint16_t actual = event[10] | (static_cast<uint16_t>(event[11]) << 8);
    fprintf(stderr, "remote: HCI version %u, manufacturer %u\n", event[6], actual);
    if (actual != manufacturer) {
        errno = ENODEV;
        return -1;
    }
    if (hci_command(fd, 0x0c03, nullptr, 0, event) < 0) return -1;

    if (manufacturer == 70) {
        constexpr std::array<uint8_t, 8> event_mask{0xff, 0xff, 0xff, 0xff,
                                                    0xff, 0xff, 0xff, 0x3f};
        constexpr std::array<uint8_t, 8> le_mask{0x02};
        if (hci_command(fd, 0x0c01, event_mask.data(), event_mask.size(), event) < 0 ||
            hci_command(fd, 0x2001, le_mask.data(), le_mask.size(), event) < 0)
            return -1;
    }
    constexpr std::array<uint8_t, 7> parameters{1, 0x80, 0, 0x36, 0, 0, 0};
    constexpr std::array<uint8_t, 2> enable{1, 0};
    return hci_command(fd, 0x200b, parameters.data(), parameters.size(), event) < 0 ||
                   hci_command(fd, 0x200c, enable.data(), enable.size(), event) < 0
               ? -1
               : 0;
}

void hci_stop_scan(int fd) {
    constexpr std::array<uint8_t, 6> disable{1, 0x0c, 0x20, 2, 0, 0};
    (void)write_all(fd, disable.data(), disable.size());
}

}  // namespace xiaomi::ble_remote
