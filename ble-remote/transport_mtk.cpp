/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/file.h>

#include "transport.h"

static bool probe(void) {
    /* The module can be present before USB enumeration creates /dev/stpbt. */
    return access("/dev/stpbt", F_OK) == 0 || access("/sys/module/btmtk_usb", F_OK) == 0;
}

static int open_device(void) {
    int64_t deadline = now_ms() + 20000;
    do {
        int fd = open("/dev/stpbt", O_RDWR | O_CLOEXEC | O_NONBLOCK);
        if (fd >= 0) {
            if (flock(fd, LOCK_EX | LOCK_NB) == 0) {
                return fd;
            }

            int error = errno;
            close(fd);
            errno = error;
            return -1;
        }

        if (errno != ENOENT && errno != EAGAIN && errno != ENODEV && errno != EBUSY) {
            return -1;
        }
        if (stopping) break;
        usleep(200000);
    } while (now_ms() < deadline);
    errno = stopping ? EINTR : ETIMEDOUT;
    return -1;
}

static void close_device(int fd) {
    close(fd);
}

const remote_transport mtk_transport = {
        "MediaTek USB", 70, probe, open_device, close_device,
};
