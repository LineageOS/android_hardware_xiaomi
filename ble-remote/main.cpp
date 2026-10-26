/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include <android-base/unique_fd.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <initializer_list>

#include "remote_hci.h"
#include "remote_input.h"
#include "remote_protocol.h"
#include "transport.h"

namespace xiaomi::ble_remote {

volatile sig_atomic_t stopping = 0;

int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

int write_all(int fd, const void* data, size_t size) {
    auto* bytes = static_cast<const uint8_t*>(data);
    while (size) {
        const ssize_t count = write(fd, bytes, size);
        if (count < 0 && errno == EINTR && !stopping) continue;
        if (count <= 0) {
            if (count == 0) errno = EIO;
            return -1;
        }
        bytes += count;
        size -= static_cast<size_t>(count);
    }
    return 0;
}

namespace {

void stop(int) {
    stopping = 1;
}

class ControllerLock {
  public:
    ControllerLock()
        : fd_(open("/tmp/xiaomi-recovery-remote.lock", O_CREAT | O_RDWR | O_CLOEXEC, 0600)) {
        if (fd_.get() >= 0 && flock(fd_.get(), LOCK_EX | LOCK_NB) < 0) fd_.reset();
    }
    explicit operator bool() const { return fd_.get() >= 0; }

  private:
    android::base::unique_fd fd_;
};

int scan(int fd) {
    RemoteInput input = RemoteInput::create();
    if (!input) return -1;

    fprintf(stderr, "ble-remote: scanning for MI RC\n");
    HciEvent event{};
    uint8_t last_key = 0;
    std::array<uint8_t, 6> last_address{};
    int64_t last_seen = 0;

    while (!stopping) {
        int length = hci_read_event(fd, event, now_ms() + 1000);
        if (length < 0) {
            if (errno == ETIMEDOUT) continue;
            return -1;
        }
        if (length < 4 || event[0] != 0x3e || event[2] != 0x02) continue;

        size_t position = 4;
        for (unsigned report = 0; report < event[3]; ++report) {
            // Legacy LE report: type, address type, address, data length, data, RSSI.
            if (position > static_cast<size_t>(length) || length - position < 10) break;
            const size_t data_size = event[position + 8];
            if (data_size > static_cast<size_t>(length) - position - 10) break;

            const uint8_t key = remote_key(event.data() + position + 9, data_size);
            if (key) {
                const int64_t now = now_ms();
                const bool new_press = key != last_key ||
                                       memcmp(last_address.data(), event.data() + position + 2,
                                              last_address.size()) != 0 ||
                                       now - last_seen >= 300;
                if (new_press && input.inject(key) < 0) return -1;
                last_key = key;
                memcpy(last_address.data(), event.data() + position + 2, last_address.size());
                last_seen = now;
            }
            position += data_size + 10;
        }
    }
    return 0;
}

TransportSession open_transport() {
    for (Transport* transport : {&mtk_transport(), &bcm_transport()}) {
        if (!transport->probe()) continue;
        fprintf(stderr, "ble-remote: probing %.*s\n", static_cast<int>(transport->name().size()),
                transport->name().data());
        const int fd = transport->open();
        if (fd < 0) {
            fprintf(stderr, "ble-remote: transport open failed: %s\n", strerror(errno));
            return {};
        }
        return TransportSession(*transport, fd);
    }
    fprintf(stderr, "ble-remote: no supported Bluetooth transport found\n");
    return {};
}

}  // namespace
}  // namespace xiaomi::ble_remote

int main(int argc, char**) {
    using namespace xiaomi::ble_remote;
    if (argc != 1) {
        fprintf(stderr, "usage: ble-remote-scanner\n");
        return 1;
    }
    if (access("/system/bin/recovery", F_OK) && access("/sbin/recovery", F_OK)) {
        fprintf(stderr, "ble-remote: recovery-only service\n");
        return 1;
    }

    signal(SIGTERM, stop);
    signal(SIGINT, stop);
    setvbuf(stderr, nullptr, _IONBF, 0);

    ControllerLock lock;
    if (!lock) {
        perror("ble-remote: acquire controller lock");
        return 1;
    }

    TransportSession session = open_transport();
    if (!session) return 1;
    fprintf(stderr, "ble-remote: using %.*s\n", static_cast<int>(session.transport().name().size()),
            session.transport().name().data());

    bool scanning = false;
    int result = 1;
    if (hci_start_scan(session.get(), session.transport().manufacturer()) < 0) goto done;
    scanning = true;
    result = scan(session.get()) == 0 ? 0 : 1;

done:
    if (result && !stopping) fprintf(stderr, "ble-remote: stopped: %s\n", strerror(errno));
    if (scanning) hci_stop_scan(session.get());
    return result;
}
