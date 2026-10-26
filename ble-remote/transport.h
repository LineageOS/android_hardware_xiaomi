/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <signal.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

namespace xiaomi::ble_remote {

extern volatile sig_atomic_t stopping;

class Transport {
  public:
    virtual ~Transport() = default;
    virtual std::string_view name() const = 0;
    virtual uint16_t manufacturer() const = 0;
    virtual bool probe() const = 0;
    virtual int open() = 0;
    virtual void close(int fd) = 0;
};

Transport& mtk_transport();
Transport& bcm_transport();

int64_t now_ms();
int write_all(int fd, const void* data, size_t size);

class TransportSession {
  public:
    TransportSession() = default;
    TransportSession(Transport& transport, int fd) : transport_(&transport), fd_(fd) {}
    ~TransportSession() { reset(); }

    TransportSession(const TransportSession&) = delete;
    TransportSession& operator=(const TransportSession&) = delete;

    TransportSession(TransportSession&& other) noexcept { swap(other); }
    TransportSession& operator=(TransportSession&& other) noexcept {
        if (this != &other) {
            reset();
            swap(other);
        }
        return *this;
    }

    explicit operator bool() const { return fd_ >= 0; }
    int get() const { return fd_; }
    Transport& transport() const { return *transport_; }
    void reset() {
        if (fd_ >= 0) transport_->close(fd_);
        transport_ = nullptr;
        fd_ = -1;
    }

  private:
    void swap(TransportSession& other) noexcept {
        std::swap(transport_, other.transport_);
        std::swap(fd_, other.fd_);
    }

    Transport* transport_ = nullptr;
    int fd_ = -1;
};

}  // namespace xiaomi::ble_remote
