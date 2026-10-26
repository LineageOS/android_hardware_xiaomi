/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <unistd.h>

#include "remote_hci.h"
#include "remote_input.h"
#include "remote_protocol.h"
#include "transport.h"

volatile sig_atomic_t stopping;

static void stop(int signal_number) {
    (void)signal_number;
    stopping = 1;
}

int64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int write_all(int fd, const void* buffer, size_t size) {
    const auto* p = static_cast<const uint8_t*>(buffer);
    while (size) {
        ssize_t n = write(fd, p, size);
        if (n < 0 && errno == EINTR && !stopping) continue;
        if (n <= 0) {
            if (!n) errno = EIO;
            return -1;
        }
        p += n;
        size -= n;
    }
    return 0;
}

static int scan(int fd, bool observe) {
    int input = -1;
    if (!observe && (input = remote_open_input()) < 0) return -1;
    fprintf(stderr, "remote: scanning for MI RC (%s)\n",
            observe ? "observe only" : "input enabled");
    uint8_t event[257], last_key = 0, last_address[6] = {0};
    int64_t last_seen = 0;
    int result = -1;
    while (!stopping) {
        int n = hci_read_event(fd, event, now_ms() + 1000);
        if (n < 0) {
            if (errno == ETIMEDOUT) continue;
            break;
        }
        if (n < 4 || event[0] != 0x3e || event[2] != 0x02) continue;
        size_t pos = 4;
        for (unsigned report = 0; report < event[3]; ++report) {
            /* Legacy LE report: event type, address type, address[6],
             * data length, data, RSSI. */
            if (pos > (size_t)n || (size_t)n - pos < 10) break;
            size_t len = event[pos + 8];
            if (len > (size_t)n - pos - 10) break;
            uint8_t key = remote_key(event + pos + 9, len);
            if (key) {
                int64_t now = now_ms();
                if (key != last_key || memcmp(last_address, event + pos + 2, 6) ||
                    now - last_seen >= 300) {
                    if (observe)
                        fprintf(stderr, "remote: observed wire key %02x\n", key);
                    else if (remote_inject_key(input, key))
                        goto done;
                }
                last_key = key;
                memcpy(last_address, event + pos + 2, 6);
                last_seen = now;
            }
            pos += len + 10;
        }
    }
    if (stopping) result = 0;
done:
    if (input >= 0) remote_close_input(input);
    return result;
}

int main(int argc, char** argv) {
    bool observe = argc == 2 && !strcmp(argv[1], "--observe");
    if (argc > 1 && !observe) {
        fprintf(stderr, "usage: %s [--observe]\n", argv[0]);
        return 1;
    }
    if (access("/system/bin/recovery", F_OK) && access("/sbin/recovery", F_OK)) {
        fprintf(stderr, "remote: recovery-only service\n");
        return 1;
    }
    int lock = open("/tmp/jaws-recovery-remote.lock", O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB)) {
        perror("remote: another scanner may already be running");
        if (lock >= 0) close(lock);
        return 1;
    }
    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    setvbuf(stderr, NULL, _IONBF, 0);

    const struct remote_transport* const transports[] = {&mtk_transport, &bcm_transport};
    const struct remote_transport* selected = NULL;
    int fd = -1;
    for (size_t i = 0; i < sizeof(transports) / sizeof(transports[0]); ++i) {
        const struct remote_transport* candidate = transports[i];
        if (!candidate->probe()) continue;
        fprintf(stderr, "remote: probing %s\n", candidate->name);
        fd = candidate->open();
        if (fd < 0) {
            fprintf(stderr, "remote: %s open failed: %s\n", candidate->name, strerror(errno));
            /* A present transport must not be mistaken for another controller. */
            close(lock);
            return 1;
        }
        selected = candidate;
        break;
    }
    if (!selected) {
        fprintf(stderr, "remote: no supported Bluetooth transport found\n");
        close(lock);
        return 1;
    }
    fprintf(stderr, "remote: using %s\n", selected->name);
    int result = 1;
    bool scanning = false;
    if (hci_start_scan(fd, selected->manufacturer) < 0) goto done;
    scanning = true;
    result = scan(fd, observe) == 0 ? 0 : 1;
done:
    if (result && !stopping) fprintf(stderr, "remote: stopped: %s\n", strerror(errno));
    if (scanning) hci_stop_scan(fd);
    selected->close(fd);
    close(lock);
    return result;
}
