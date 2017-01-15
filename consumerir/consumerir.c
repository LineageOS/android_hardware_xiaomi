/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define LOG_TAG "ConsumerIrLirc"

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <linux/lirc.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <hardware/hardware.h>
#include <hardware/consumerir.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static const consumerir_freq_range_t consumerir_freqs[] = {
    {.min = 30000, .max = 60000},
};

static int open_lircdev(void)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("ro.lirc.dev", value, "/dev/lirc0");
    int fd = open(value, O_RDWR);
    if (fd < 0) {
        ALOGE("failed to open %s error %d", value, fd);
        return fd;
    }

    return fd;
}

static int consumerir_transmit(struct consumerir_device *dev __unused,
   int carrier_freq, const int pattern[], int pattern_len)
{
    int total_time = 0;
    long i;
    int fd;
    int rc;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int duty_cycle;

    fd = open_lircdev();
    if (fd < 0) {
        return fd;
    }

    rc = ioctl(fd, LIRC_SET_SEND_CARRIER, &carrier_freq);
    if (rc < 0) {
        ALOGE("failed to set carrier %d error %d", carrier_freq, rc);
        goto out_close;
    }

    property_get("ro.lirc.duty_cycle", value, "33");
    duty_cycle = atoi(value);
    rc = ioctl(fd, LIRC_SET_SEND_DUTY_CYCLE, &duty_cycle);
    if (rc < 0) {
        ALOGE("failed to set duty cycle %d error %d", duty_cycle, rc);
        goto out_close;
    }

    if (pattern_len&1) {
        rc = write(fd, pattern, sizeof(*pattern)*pattern_len);
    }
    else {
        rc = write(fd, pattern, sizeof(*pattern)*(pattern_len-1));
        usleep(pattern[pattern_len-1]);
    }
    if (rc < 0) {
        ALOGE("failed to write pattern %d error %d", pattern_len, rc);
        goto out_close;
    }

    rc = 0;

out_close:
    close(fd);

    return rc;
}

static int consumerir_get_num_carrier_freqs(struct consumerir_device *dev __unused)
{
    return ARRAY_SIZE(consumerir_freqs);
}

static int consumerir_get_carrier_freqs(struct consumerir_device *dev __unused,
    size_t len, consumerir_freq_range_t *ranges)
{
    size_t to_copy = ARRAY_SIZE(consumerir_freqs);

    to_copy = len < to_copy ? len : to_copy;
    memcpy(ranges, consumerir_freqs, to_copy * sizeof(consumerir_freq_range_t));
    return to_copy;
}

static int consumerir_close(hw_device_t *dev)
{
    free(dev);
    return 0;
}

/*
 * Generic device handling
 */
static int consumerir_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    if (strcmp(name, CONSUMERIR_TRANSMITTER) != 0) {
        return -EINVAL;
    }
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    int lircfd = open_lircdev();
    if (lircfd < 0) {
        return lircfd;
    }
    close(lircfd);

    consumerir_device_t *dev = malloc(sizeof(consumerir_device_t));
    memset(dev, 0, sizeof(consumerir_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = consumerir_close;

    dev->transmit = consumerir_transmit;
    dev->get_num_carrier_freqs = consumerir_get_num_carrier_freqs;
    dev->get_carrier_freqs = consumerir_get_carrier_freqs;

    *device = (hw_device_t*) dev;
    return 0;
}

static struct hw_module_methods_t consumerir_module_methods = {
    .open = consumerir_open,
};

consumerir_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = CONSUMERIR_MODULE_API_VERSION_1_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = CONSUMERIR_HARDWARE_MODULE_ID,
        .name               = "LIRC IR HAL",
        .author             = "Xiaomi Corporation",
        .methods            = &consumerir_module_methods,
    },
};
