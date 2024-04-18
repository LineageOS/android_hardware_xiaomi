/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace aidl {
namespace android {
namespace hardware {
namespace light {

/**
 * Interface for dumpable objects using AIDL dump()'s file descriptor.
 */
class IDumpable {
  public:
    virtual ~IDumpable() = default;

    /**
     * Write information regarding this object to the given file descriptor using dprintf().
     * You should avoid ending newline, since the caller will add it.
     * @param fd The file descriptor to write to.
     */
    virtual void dump(int fd) const = 0;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
