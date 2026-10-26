/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <cstring>
#include <string_view>

#include "transport.h"

namespace xiaomi::ble_remote {
namespace {

class BroadcomTransport final : public Transport {
  public:
    std::string_view name() const override { return "Broadcom UART"; }
    uint16_t manufacturer() const override { return 15; }

    bool probe() const override {
        return access("/dev/ttyS1", F_OK) == 0 &&
               access("/sys/module/bt_device/parameters/btpower_evt", F_OK) == 0 &&
               access("/sys/class/rfkill/rfkill0/state", F_OK) == 0;
    }

    int open() override {
        const int fd = ::open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_CLOEXEC | O_NONBLOCK);
        if (fd < 0) return -1;
        termios original{};
        bool saved = false;
        bool exclusive = false;
        const auto fail = [&]() {
            const int error = errno;
            if (saved) tcsetattr(fd, TCSANOW, &original);
            if (exclusive) ioctl(fd, TIOCNXCL);
            ::close(fd);
            errno = stopping ? EINTR : error;
            return -1;
        };

        if (flock(fd, LOCK_EX | LOCK_NB) < 0 || ioctl(fd, TIOCEXCL) < 0) return fail();
        exclusive = true;
        if (tcgetattr(fd, &original) < 0) return fail();
        saved = true;
        termios tty = original;
        cfmakeraw(&tty);
        tty.c_cflag = (tty.c_cflag & ~CRTSCTS) | CLOCAL | CREAD;
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 0;
        if (cfsetispeed(&tty, B115200) < 0 || cfsetospeed(&tty, B115200) < 0 ||
            tcsetattr(fd, TCSANOW, &tty) < 0)
            return fail();
        if (write_sysfs("/sys/module/bt_device/parameters/btpower_evt", "1") < 0 ||
            write_sysfs("/sys/class/rfkill/rfkill0/state", "0") < 0)
            return fail();
        usleep(200000);
        if (stopping || write_sysfs("/sys/class/rfkill/rfkill0/state", "1") < 0) return fail();
        usleep(200000);
        if (stopping || tcflush(fd, TCIOFLUSH) < 0) return fail();
        saved_original_ = original;
        have_original_ = saved;
        exclusive_ = exclusive;
        return fd;
    }

    void close(int fd) override {
        if (have_original_) tcsetattr(fd, TCSANOW, &saved_original_);
        if (exclusive_) ioctl(fd, TIOCNXCL);
        have_original_ = false;
        exclusive_ = false;
        ::close(fd);
    }

  private:
    static int write_sysfs(const char* path, const char* value) {
        const int fd = ::open(path, O_WRONLY | O_CLOEXEC);
        if (fd < 0) return -1;
        const int result = write_all(fd, value, strlen(value));
        const int error = errno;
        ::close(fd);
        errno = error;
        return result;
    }

    termios saved_original_{};
    bool have_original_ = false;
    bool exclusive_ = false;
};

}  // namespace

Transport& bcm_transport() {
    static BroadcomTransport transport;
    return transport;
}

}  // namespace xiaomi::ble_remote
