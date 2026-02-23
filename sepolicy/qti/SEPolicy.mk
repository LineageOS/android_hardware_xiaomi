#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

SEPOLICY_PLATFORM := $(subst device/qcom/sepolicy_vndr/,,$(SEPOLICY_PATH))

BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/vendor

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/private

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/public

ifneq ($(SEPOLICY_PLATFORM), legacy-um)
BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/vendor/common-um

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/private/common-um

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/qti/public/common-um
endif
