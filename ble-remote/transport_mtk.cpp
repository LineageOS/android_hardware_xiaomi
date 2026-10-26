/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <string_view>

#include "remote_hci.h"
#include "transport.h"

namespace xiaomi::ble_remote {
namespace {

class MediaTekTransport final : public Transport {
  public:
    std::string_view name() const override { return "MediaTek USB"; }
    uint16_t manufacturer() const override { return kMediaTekManufacturerId; }

    bool probe() const override {
        // The driver can load before USB enumeration creates /dev/stpbt.
        return access("/dev/stpbt", F_OK) == 0 || access("/sys/module/btmtk_usb", F_OK) == 0;
    }

    int open() override {
        const int64_t deadline = now_ms() + 20000;
        do {
            const int fd = ::open("/dev/stpbt", O_RDWR | O_CLOEXEC | O_NONBLOCK);
            if (fd >= 0) return fd;
            if (errno != ENOENT && errno != EAGAIN && errno != ENODEV && errno != EBUSY) return -1;
            if (stopping) break;
            usleep(200000);
        } while (now_ms() < deadline);

        errno = stopping ? EINTR : ETIMEDOUT;
        return -1;
    }

    void close(int fd) override { ::close(fd); }
};

}  // namespace

Transport& mtk_transport() {
    static MediaTekTransport transport;
    return transport;
}

}  // namespace xiaomi::ble_remote
