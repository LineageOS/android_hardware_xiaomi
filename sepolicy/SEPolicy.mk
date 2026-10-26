#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

SYSTEM_EXT_PRIVATE_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/system_ext/private

SYSTEM_EXT_PUBLIC_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/system_ext/public

BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/common/vendor

ifeq ($(PRODUCT_IS_ATV), true)
BOARD_VENDOR_SEPOLICY_DIRS += \
    hardware/xiaomi/sepolicy/atv/vendor
endif
