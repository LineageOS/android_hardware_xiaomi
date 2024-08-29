/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "sensors.udfps"

#include <errno.h>
#include <fcntl.h>
#include <hardware/sensors.h>
#include <log/log.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <utils/SystemClock.h>

static const char *udfps_state_paths[] = {
        "/sys/devices/virtual/touch/tp_dev/fp_state",
        "/sys/touchpanel/fp_state",
        NULL,
};

static struct sensor_t udfps_sensor = {
        .name = "UDFPS Sensor",
        .vendor = "The LineageOS Project",
        .version = 1,
        .handle = 0,
        .type = SENSOR_TYPE_DEVICE_PRIVATE_BASE + 1,
        .maxRange = 2048.0f,
        .resolution = 1.0f,
        .power = 0,
        .minDelay = -1,
        .fifoReservedEventCount = 0,
        .fifoMaxEventCount = 0,
        .stringType = "org.lineageos.sensor.udfps",
        .requiredPermission = "",
        .maxDelay = 0,
        .flags = SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP,
        .reserved = {},
};

struct udfps_context_t {
    sensors_poll_device_1_t device;
    int fd;
};

static int udfps_read_line(int fd, char* buf, size_t len) {
    int rc;

    rc = lseek(fd, 0, SEEK_SET);
    if (rc < 0) {
        ALOGE("Failed to seek: %d", -errno);
        return rc;
    }

    rc = read(fd, buf, len);
    if (rc < 0) {
        ALOGE("Failed to read: %d", -errno);
        return rc;
    }

    return rc;
}

static int udfps_read_state(int fd, int& pos_x, int& pos_y) {
    int rc, state = 0;
    char buf[64];

    rc = udfps_read_line(fd, buf, sizeof(buf));
    if (rc > 0) {
        rc = sscanf(buf, "%d,%d,%d", &pos_x, &pos_y, &state);
        if (rc != 3) {
            ALOGE("Failed to parse fp_state: %d", rc);
            state = 0;
        }
    }

    return state;
}

static int udfps_wait_event(int fd, int timeout) {
    struct pollfd fds = {
            .fd = fd,
            .events = POLLERR | POLLPRI,
            .revents = 0,
    };
    int rc;

    do {
        rc = poll(&fds, 1, timeout);
    } while (rc < 0 && errno == EINTR);

    return rc;
}

static void udfps_flush_events(int fd) {
    char buf[64];

    while (udfps_wait_event(fd, 0) > 0) {
        udfps_read_line(fd, buf, sizeof(buf));
    }
}

static int udfps_close(struct hw_device_t* dev) {
    udfps_context_t* ctx = reinterpret_cast<udfps_context_t*>(dev);

    if (ctx) {
        close(ctx->fd);
        delete ctx;
    }

    return 0;
}

static int udfps_activate(struct sensors_poll_device_t* dev, int handle, int enabled) {
    udfps_context_t* ctx = reinterpret_cast<udfps_context_t*>(dev);

    if (!ctx || handle) {
        return -EINVAL;
    }

    // Flush any pending events
    if (enabled) udfps_flush_events(ctx->fd);

    return 0;
}

static int udfps_setDelay(struct sensors_poll_device_t* dev, int handle, int64_t /* ns */) {
    udfps_context_t* ctx = reinterpret_cast<udfps_context_t*>(dev);

    if (!ctx || handle) {
        return -EINVAL;
    }

    return 0;
}

static int udfps_poll(struct sensors_poll_device_t* dev, sensors_event_t* data, int /* count */) {
    udfps_context_t* ctx = reinterpret_cast<udfps_context_t*>(dev);

    if (!ctx) {
        return -EINVAL;
    }

    int fod_x, fod_y, fod_state = 0;

    do {
        int rc = udfps_wait_event(ctx->fd, -1);
        if (rc < 0) {
            ALOGE("Failed to poll fp_state: %d", -errno);
            return -errno;
        } else if (rc > 0) {
            fod_state = udfps_read_state(ctx->fd, fod_x, fod_y);
        }
    } while (!fod_state);

    memset(data, 0, sizeof(sensors_event_t));
    data->version = sizeof(sensors_event_t);
    data->sensor = udfps_sensor.handle;
    data->type = udfps_sensor.type;
    data->timestamp = ::android::elapsedRealtimeNano();
    data->data[0] = fod_x;
    data->data[1] = fod_y;

    return 1;
}

static int udfps_batch(struct sensors_poll_device_1* /* dev */, int /* handle */, int /* flags */,
                       int64_t /* period_ns */, int64_t /* max_ns */) {
    return 0;
}

static int udfps_flush(struct sensors_poll_device_1* /* dev */, int /* handle */) {
    return -EINVAL;
}

static int open_sensors(const struct hw_module_t* module, const char* /* name */,
                        struct hw_device_t** device) {
    udfps_context_t* ctx = new udfps_context_t();

    memset(ctx, 0, sizeof(udfps_context_t));
    ctx->device.common.tag = HARDWARE_DEVICE_TAG;
    ctx->device.common.version = SENSORS_DEVICE_API_VERSION_1_3;
    ctx->device.common.module = const_cast<hw_module_t*>(module);
    ctx->device.common.close = udfps_close;
    ctx->device.activate = udfps_activate;
    ctx->device.setDelay = udfps_setDelay;
    ctx->device.poll = udfps_poll;
    ctx->device.batch = udfps_batch;
    ctx->device.flush = udfps_flush;

    for (int i = 0; udfps_state_paths[i]; i++) {
        ctx->fd = open(udfps_state_paths[i], O_RDONLY);
        if (ctx->fd >= 0) {
            break;
        }
    }

    if (ctx->fd < 0) {
        ALOGE("Failed to open fp state: %d", -errno);
        delete ctx;

        return -ENODEV;
    }

    *device = &ctx->device.common;

    return 0;
}

static struct hw_module_methods_t udfps_module_methods = {
        .open = open_sensors,
};

static int udfps_get_sensors_list(struct sensors_module_t*, struct sensor_t const** list) {
    *list = &udfps_sensor;

    return 1;
}

static int udfps_set_operation_mode(unsigned int mode) {
    return !mode ? 0 : -EINVAL;
}

struct sensors_module_t HAL_MODULE_INFO_SYM = {
        .common = {.tag = HARDWARE_MODULE_TAG,
                   .version_major = 1,
                   .version_minor = 0,
                   .id = SENSORS_HARDWARE_MODULE_ID,
                   .name = "UDFPS Sensor module",
                   .author = "Ivan Vecera",
                   .methods = &udfps_module_methods,
                   .dso = NULL,
                   .reserved = {0}},
        .get_sensors_list = udfps_get_sensors_list,
        .set_operation_mode = udfps_set_operation_mode,
};
