/*
 * SPDX-FileCopyrightText: 2024 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include "XiaomiSmartPen.h"

#define LOG_TAG "XiaomiSmartPen"

#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/stringprintf.h>
#include "../Utils.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace accessories {

static const std::string kPenEnabledPath = "/sys/touchpanel/pen";

static const std::string kWirelessPath = "/sys/class/power_supply/wireless/";
static const std::string kPenChargeStatePath = kWirelessPath + "reverse_pen_chg_state";
static const std::string kPenMacPath = kWirelessPath + "pen_mac";
static const std::string kPenSocPath = kWirelessPath + "reverse_pen_soc";

static const std::string kWirelessUeventMatch = "POWER_SUPPLY_NAME=wireless";
static const std::string kPenChargeStateKey = "POWER_SUPPLY_REVERSE_PEN_CHG_STATE";
static const std::string kPenMacKey = "POWER_SUPPLY_PEN_MAC";
static const std::string kPenSocKey = "POWER_SUPPLY_REVERSE_PEN_SOC";

// Reverse charging is active, i.e. the pen is attached
static constexpr int kPenChargeStateCharging = 4;

static constexpr uint64_t kMaxMac = 0xFFFFFFFFFFFF;

// The kernel prints the address as a plain hex number, without leading zeroes
static std::string parsePenMac(const std::string& value) {
    uint64_t mac;
    if (value.empty() || !::android::base::ParseUint("0x" + value, &mac, kMaxMac) || mac == 0) {
        return "";
    }

    return ::android::base::StringPrintf(
            "%02X:%02X:%02X:%02X:%02X:%02X", static_cast<unsigned>((mac >> 40) & 0xFF),
            static_cast<unsigned>((mac >> 32) & 0xFF), static_cast<unsigned>((mac >> 24) & 0xFF),
            static_cast<unsigned>((mac >> 16) & 0xFF), static_cast<unsigned>((mac >> 8) & 0xFF),
            static_cast<unsigned>(mac & 0xFF));
}

static std::string getValue(const UeventListener::Environment& environment,
                            const std::string& key) {
    auto it = environment.find(key);
    return it != environment.end() ? it->second : "";
}

XiaomiSmartPen::XiaomiSmartPen()
    : mWirelessListener(kWirelessUeventMatch,
                        [this](const UeventListener::Environment& environment) {
                            onWirelessChanged(getValue(environment, kPenChargeStateKey),
                                              getValue(environment, kPenMacKey),
                                              getValue(environment, kPenSocKey));
                        }) {
    mAccessoryInfo = {
            .id = "active_pen:internal:1",
            .type = AccessoryType::ACTIVE_PEN,
            .isPersistent = true,
            .displayName = "Xiaomi Smart Pen",
            .displayId = "",
            .supportsToggling = true,
            .supportsBatteryQuerying = true,
            .isCharging = false,
            .dataLink = std::nullopt,
    };

    onEnabledChanged();

    mWirelessListener.start();

    // The pen might have been attached before we started listening
    std::string chargeState, mac, soc;
    readFromFile(kPenChargeStatePath, chargeState);
    readFromFile(kPenMacPath, mac);
    readFromFile(kPenSocPath, soc);
    onWirelessChanged(chargeState, mac, soc);
}

::ndk::ScopedAStatus XiaomiSmartPen::setEnabled(bool enabled) {
    std::lock_guard lock(mDataMutex);

    if (!writeToFile(kPenEnabledPath, enabled ? "1" : "0")) {
        LOG(ERROR) << "Failed to write to " << kPenEnabledPath;
        return ::ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
    }

    mEnabled = enabled;

    updateStatus();

    return ::ndk::ScopedAStatus::ok();
}

void XiaomiSmartPen::updateStatus() {
    mAccessoryStatus = mEnabled ? AccessoryStatus::ENABLED : AccessoryStatus::DISABLED;

    // The battery level is only known while the pen is being charged
    if (mAccessoryInfo.isCharging && mBatteryLevel != BatteryInfo::BATTERY_LEVEL_UNKNOWN) {
        mBatteryInfo = {
                .status = BatteryStatus::CHARGING,
                .levelPercentage = mBatteryLevel,
        };
    } else {
        mBatteryInfo = {
                .status = BatteryStatus::UNKNOWN,
                .levelPercentage = BatteryInfo::BATTERY_LEVEL_UNKNOWN,
        };
    }

    onAccessoryStatusUpdated();
}

void XiaomiSmartPen::onEnabledChanged() {
    std::lock_guard lock(mDataMutex);

    if (!readFromFile(kPenEnabledPath, mEnabled)) {
        LOG(ERROR) << "Failed to read from " << kPenEnabledPath;
        return;
    }

    updateStatus();
}

void XiaomiSmartPen::onWirelessChanged(const std::string& chargeState, const std::string& mac,
                                       const std::string& soc) {
    std::lock_guard lock(mDataMutex);

    int state;
    if (!::android::base::ParseInt(chargeState, &state)) {
        // Not a pen related update
        return;
    }

    bool attached = state == kPenChargeStateCharging;

    int level;
    mBatteryLevel = ::android::base::ParseInt(soc, &level, 0, 100)
                            ? level
                            : BatteryInfo::BATTERY_LEVEL_UNKNOWN;

    // The address is kept once the pen is detached, but it's only meaningful while attached
    std::string address = parsePenMac(mac);
    bool infoChanged = attached != mAccessoryInfo.isCharging;
    if (attached && !address.empty() &&
        (!mAccessoryInfo.dataLink || mAccessoryInfo.dataLink->id != address)) {
        mAccessoryInfo.dataLink = DataLinkInfo{
                .type = DataLinkType::BLUETOOTH,
                .id = address,
        };
        infoChanged = true;
    }

    if (infoChanged) {
        LOG(INFO) << "Pen "
                  << (!attached                 ? std::string("detached")
                      : mAccessoryInfo.dataLink ? "attached: " + mAccessoryInfo.dataLink->id
                                                : std::string("attached, address unknown"));
        mAccessoryInfo.isCharging = attached;
        onAccessoryInfoUpdated();
    }

    updateStatus();
}

}  // namespace accessories
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
