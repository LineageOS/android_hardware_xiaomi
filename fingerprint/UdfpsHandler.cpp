/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "UdfpsHandler.h"
#include <dlfcn.h>

#define UDFPS_HANDLER_LIB_NAME "libudfpshandler.so"
#define UDFPS_HANDLER_FACTORY "UDFPS_HANDLER_FACTORY"

UdfpsHandlerFactory* getUdfpsHandlerFactory() {
    void* libudfpshander;
    UdfpsHandlerFactory* factory_handler;

    libudfpshander = dlopen(UDFPS_HANDLER_LIB_NAME, RTLD_LAZY);
    if (!libudfpshander) {
        goto error;
    }

    factory_handler = (UdfpsHandlerFactory*)dlsym(libudfpshander, UDFPS_HANDLER_FACTORY);
    if (!factory_handler) {
        goto error;
    }

    return factory_handler;

error:
    if (libudfpshander) {
        dlclose(libudfpshander);
    }

    return nullptr;
}
