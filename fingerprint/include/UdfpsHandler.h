/*
 * Copyright (C) 2022,2025 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#ifndef DEVICE_USES_NEW_IMPLEMENTATION
#include "fingerprint-legacy.h"
#else
#include "fingerprint.h"
#endif

class UdfpsHandler {
  public:
    virtual ~UdfpsHandler() = default;

    virtual void init(fingerprint_device_t* device) {};
    virtual void onFingerDown(uint32_t x, uint32_t y, float minor, float major) {};
    virtual void onFingerUp() {};

    virtual void onAcquired(int32_t result, int32_t vendorCode) {};
    virtual void onAuthenticationSucceeded() {};
    virtual void onAuthenticationFailed() {};
    virtual void cancel() {};
};

struct UdfpsHandlerFactory {
    UdfpsHandler* (*create)();
    void (*destroy)(UdfpsHandler* handler);
};

UdfpsHandlerFactory* getUdfpsHandlerFactory();
