#
# Copyright (C) 2021 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifneq (,$(findstring hardware/google/interfaces, $(PRODUCT_SOONG_NAMESPACES)))
ifneq (,$(findstring hardware/google/pixel, $(PRODUCT_SOONG_NAMESPACES)))

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES := \
    android.hardware.power-ndk_platform \
    libbase \
    libbinder_ndk \
    libcutils \
    libdl \
    liblog \
    libperfmgr \
    libutils \
    pixel-power-ext-ndk_platform

LOCAL_SRC_FILES := \
    service.cpp \
    InteractionHandler.cpp \
    Power.cpp \
    PowerExt.cpp

LOCAL_CFLAGS := -Wno-unused-parameter -Wno-unused-variable

ifneq ($(TARGET_POWERHAL_MODE_EXT),)
    LOCAL_CFLAGS += -DMODE_EXT
    LOCAL_SRC_FILES += ../../../../$(TARGET_POWERHAL_MODE_EXT)
endif

LOCAL_MODULE := android.hardware.power-service.xiaomi-libperfmgr
LOCAL_INIT_RC := android.hardware.power-service.xiaomi-libperfmgr.rc
LOCAL_MODULE_TAGS := optional
LOCAL_VENDOR_MODULE := true
LOCAL_VINTF_FRAGMENTS := android.hardware.power-service.xiaomi.xml

include $(BUILD_EXECUTABLE)

endif
endif
