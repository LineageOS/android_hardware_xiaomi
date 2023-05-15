/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Changes from Qualcomm Innovation Center are provided under the following license:
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#define LOG_TAG "vendor.qti.vibrator.offload"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <linux/input.h>
#include <log/log.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/uevent.h>
#include <cutils/properties.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#include "include/Vibrator.h"
#include "VibratorPatterns.h"

extern "C" {
#include "libsoc_helper.h"
}

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

#define UEVENT_MSG_LEN              1024
#define SLATE_EVENT "SLATE_EVENT="
#define SLATE_EVENT_STRING_LEN      12 //length of SLATE_EVENT
/*
 * TODO Need to work on solution to get this from kernel header
 * without effecting other kernel versions where this change
 * goes in.
 */
#define SLATE_AFTER_POWER_UP        4

PatternOffload::PatternOffload()
{
    char prop_str[PROPERTY_VALUE_MAX];
    mEnabled = 0;

    if (property_get("ro.vendor.qc_aon_presence", prop_str, NULL))
        mEnabled = atoi(prop_str);

    if (mEnabled != 1)
        return;

    std::thread t(&PatternOffload::SSREventListener, this);
    t.detach();
}

void PatternOffload::SSREventListener(void)
{
    int device_fd, n, ssr_event = 0;
    char msg[UEVENT_MSG_LEN + 2];
    char *msg_ptr = msg;

    /* Offload during the bootup */
    SendPatterns();

    device_fd = uevent_open_socket(64*1024, true);
    if(device_fd < 0)
    {
        ALOGE("open socket failed: %d", device_fd);
        return;
    }

    while ((n = uevent_kernel_multicast_recv(device_fd, msg, UEVENT_MSG_LEN)) > 0) {
         if (n <= 0 || n > UEVENT_MSG_LEN) {
            ALOGE("Message length %d is not correct\n", n);
            continue;
         }
         msg[n] = '\0';
         msg[n+1] = '\0';
         if (strstr(msg, "slate_com_dev")) {
             while(*msg_ptr) {
                 if(!strncmp(msg_ptr, SLATE_EVENT, SLATE_EVENT_STRING_LEN)) {
                     msg_ptr += SLATE_EVENT_STRING_LEN;
                     ssr_event = (atoi(msg_ptr));
                     switch(ssr_event) {
                         case SLATE_AFTER_POWER_UP:
                             ALOGD("SLATE is powered up");
                             SendPatterns();
                             break;
                     }
                 }
                 while(*msg_ptr++);
             }
         }
     }
}

/** Offload patterns
 *  The sequence of steps in offloading patterns.
 *  1. Open the Glink channel to offload the patterns
 *  2. Send the configuration/meta data to co-proc
 *  3. Wait for the response from the co-proc
 *  4. Send the pattern data to co-proc
 *  5. Wait for the response
 *  6. Exit
 */
void PatternOffload::SendPatterns()
{
    uint8_t *data;
    uint32_t len;
    int32_t rc;

    rc = initChannel();
    if (rc < 0)
        return;

    rc = get_pattern_config(&data, &len);
    if (rc < 0 || !data)
        return;

    /* Send config data */
    rc = sendData(data, len);
    if (rc < 0)
        return;

    rc = get_pattern_data(&data, &len);
    if (rc < 0)
        return;

    /* Send pattern data */
    rc = sendData(data, len);
    if (rc < 0)
        ALOGE("pattern offloaded failed\n");
    else
        ALOGI("Patterns offloaded successfully\n");

    free_pattern_mem(data);
}

int PatternOffload::sendData(uint8_t *data, int len)
{
    int rc, status = 0;

    if (!data || !len)
        return -EINVAL;

    rc = GlinkCh.GlinkWrite(data, len);
    if (rc < 0)
        return rc;

    rc = GlinkCh.GlinkPoll();
    if (rc < 0)
        return rc;

    rc = GlinkCh.GlinkRead((uint8_t *)&status, 4);
    if (rc < 0)
        return rc;

    if (status != OFFLOAD_SUCCESS)
        return -EIO;

    return 0;
}


int PatternOffload::initChannel()
{
    std::string chname = "/dev/glinkpkt_slate_haptics_offload";
    int rc;

    rc = GlinkCh.GlinkOpen(chname);
    if (rc < 0)
    {
        ALOGE("Failed to open Glink channel name %s\n", chname.c_str());
        return rc;
    }

    return 0;
}

#define GLINK_MAX_CONN_RETRIES    60
int OffloadGlinkConnection::GlinkOpen(std::string& dev)
{
    int tries = GLINK_MAX_CONN_RETRIES;
    dev_name = dev;

    do {
        fd = ::open(dev_name.c_str(), O_RDWR);
        tries--;
        if (fd < 0)
        {
            ALOGE("%s: %s: open error(%s)", __func__, dev.c_str(), strerror(errno));
            sleep(1);
        }
    } while(-ETIMEDOUT == errno && tries > 0 );

    return fd;
}

int OffloadGlinkConnection::GlinkClose()
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
    return 0;
}

int OffloadGlinkConnection::GlinkPoll()
{
    ssize_t rc = 0;
    struct pollfd poll_fd;

    // wait for Rx data available in fd, for 2 seconds timeout
    poll_fd.fd = fd;
    poll_fd.events = POLLIN;

    rc = ::poll(&poll_fd, 1, 2000);

    if(rc > 0)
    {
        if (poll_fd.revents & POLLIN)
            return 0;
    } else if (rc == 0) {
           ALOGE("Glink poll timeout");
    } else {
           ALOGE("Glink poll error: %s\n", strerror(errno));
    }

    return -1;
}

int OffloadGlinkConnection::GlinkRead(uint8_t *data, size_t size)
{
    int rc = 0;
    size_t bytes_read = 0;

    if (fd < 0)
        return -1;

    if (0 != GlinkPoll())
        return -1;

    while (bytes_read < size)
    {
        rc = ::read(fd, data+bytes_read, size-bytes_read);
        if (rc < 0) {
            if (errno != EAGAIN) {
                ALOGE("%s: Read error: %s, rc %d", __func__,
                                strerror(errno), rc);
                goto read_error;
            }
        } else if (rc == 0) {
            ALOGE("%s: Zero length packet received or hardware connection went off",
                    __func__);
        }
        bytes_read += rc;
    }
    return 0;

read_error:
    return -1;
}

int OffloadGlinkConnection::GlinkWrite(uint8_t *buf, size_t buflen)
{
    size_t bytes_written_out = 0;
    int rc = 0;

    if (fd < 0)
        return -1;

    if (!buflen) {
       ALOGE("%s: Invalid buffer len", __func__);
       return 0;
    }

    while (bytes_written_out < buflen) {
        rc = ::write (fd, buf+bytes_written_out, buflen-bytes_written_out);
        if (rc < 0) {
            ALOGE("%s: Write returned failure %d", __func__, rc);
            return errno;
        }
        bytes_written_out += rc;
    }
    rc = 0;

    return rc;
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
