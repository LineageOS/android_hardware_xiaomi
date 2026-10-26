/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "transport.h"

static struct termios original;
static bool saved;

static int write_sysfs(const char* path, const char* value) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    int result = write_all(fd, value, strlen(value));
    int error = errno;
    close(fd);
    errno = error;
    return result;
}

static bool probe(void) {
    return access("/dev/ttyS1", F_OK) == 0 &&
           access("/sys/module/bt_device/parameters/btpower_evt", F_OK) == 0 &&
           access("/sys/class/rfkill/rfkill0/state", F_OK) == 0;
}

static void close_device(int fd) {
    if (saved) {
        tcsetattr(fd, TCSANOW, &original);
        saved = false;
    }
    ioctl(fd, TIOCNXCL);
    close(fd);
}

static int open_device(void) {
    int fd = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0) return -1;
    struct termios tty = {};
    if (flock(fd, LOCK_EX | LOCK_NB)) goto fail;
    if (ioctl(fd, TIOCEXCL)) goto fail;
    if (tcgetattr(fd, &original)) goto fail;
    saved = true;
    tty = original;
    cfmakeraw(&tty);
    tty.c_cflag = (tty.c_cflag & ~CRTSCTS) | CLOCAL | CREAD;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
    if (cfsetispeed(&tty, B115200) || cfsetospeed(&tty, B115200) || tcsetattr(fd, TCSANOW, &tty))
        goto fail;
    if (write_sysfs("/sys/module/bt_device/parameters/btpower_evt", "1") ||
        write_sysfs("/sys/class/rfkill/rfkill0/state", "0"))
        goto fail;
    usleep(200000);
    if (stopping || write_sysfs("/sys/class/rfkill/rfkill0/state", "1")) goto fail;
    usleep(200000);
    if (stopping || tcflush(fd, TCIOFLUSH)) goto fail;
    return fd;
fail: {
    int error = errno;
    close_device(fd);
    errno = stopping ? EINTR : error;
    return -1;
}
}

const remote_transport bcm_transport = {
        "Broadcom UART", 15, probe, open_device, close_device,
};
