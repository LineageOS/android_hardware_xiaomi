/*
 * Copyright (C) 2017-2018 The LineageOS Project
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

#define LOG_TAG "ConsumerIrSpiService"

#include <fcntl.h>
#include <linux/lirc.h>

#include <android-base/logging.h>

#include "ConsumerIr.h"

// FIXME
// This comes from linux/math.h
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 ||			\
	 (((__x) > 0) == ((__d) > 0))) ?		\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

// These comes from linux/bits.h
// FIXME: Assuming 64bit arch
#define BITS_PER_LONG 64
#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

namespace android {
namespace hardware {
namespace ir {
namespace V1_0 {
namespace implementation {

#define SPI_DEV_PATH "/dev/ir_spi"

// Using the default duty cicle from mainline ir-spi driver
// needs to be extracted somehow from the HAL
static const int dutyCycle = 50;

static const int bits = (dutyCycle * 15) / 100;
static const int32_t pulse = GENMASK(bits, 0);
static const int32_t space = 0;

static hidl_vec<ConsumerIrFreqRange> rangeVec {
    {.min = 30000, .max = 30000},
    {.min = 33000, .max = 33000},
    {.min = 36000, .max = 36000},
    {.min = 38000, .max = 38000},
    {.min = 40000, .max = 40000},
    {.min = 56000, .max = 56000},
};

static int openSpiDev() {
    int fd = open(SPI_DEV_PATH, O_RDWR);

    if (fd < 0) {
        LOG(ERROR) << "failed to open " << SPI_DEV_PATH << ", error " << fd;
    }

    return fd;
}

// Methods from ::android::hardware::ir::V1_0::IConsumerIr follow.
Return<bool> ConsumerIr::transmit(int32_t carrierFreq, const hidl_vec<int32_t>& pattern) {
    size_t entries;
    int rc = 0;
    int spiFd;
    std::vector<int32_t> buffer{carrierFreq};
    unsigned int i = 0;
    unsigned int j;
    int periods;
    int32_t val;

    spiFd = openSpiDev();
    if (spiFd < 0) {
        return spiFd;
    }

    for (const int32_t& number : pattern) {
        val = (i % 2) ? space : pulse;
        periods = DIV_ROUND_CLOSEST(number * carrierFreq, 1000000);
        if (periods < 0)
            periods = 0;
        for (j = 0; j < periods; j++)
            buffer.emplace_back(val);
        i++;
    }

    entries = buffer.size();

    if ((entries & 1) != 0) {
        rc = write(spiFd, buffer.data(), sizeof(int32_t) * entries);
    } else {
        rc = write(spiFd, buffer.data(), sizeof(int32_t) * (entries - 1));
        usleep(buffer[entries - 1]);
    }

    if (rc < 0) {
        LOG(ERROR) << "failed to write pattern " << pattern.size() << ", error: " << errno;
        goto out_close;
    }

    rc = 0;

out_close:
    close(spiFd);

    return rc == 0;
}

Return<void> ConsumerIr::getCarrierFreqs(getCarrierFreqs_cb _hidl_cb) {
    _hidl_cb(true, rangeVec);
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace ir
}  // namespace hardware
}  // namespace android
