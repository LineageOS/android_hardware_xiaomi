/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace xiaomi::ble_remote {

class RemoteInput {
  public:
    RemoteInput() = default;
    ~RemoteInput();
    RemoteInput(const RemoteInput&) = delete;
    RemoteInput& operator=(const RemoteInput&) = delete;
    RemoteInput(RemoteInput&& other) noexcept;
    RemoteInput& operator=(RemoteInput&& other) noexcept;

    static RemoteInput create();
    explicit operator bool() const { return fd_ >= 0; }
    int inject(uint8_t key) const;

  private:
    explicit RemoteInput(int fd) : fd_(fd) {}
    void reset();
    int fd_ = -1;
};

}  // namespace xiaomi::ble_remote
