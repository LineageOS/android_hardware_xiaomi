#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/vendor

ifeq ($(PRODUCT_IS_ATV), true)
BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/atv/vendor
endif

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/private

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/public
