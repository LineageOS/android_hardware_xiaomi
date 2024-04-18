/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <vector>
#include "LedDevice.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class NotificationDevice {
  public:
    virtual ~NotificationDevice() = default;

    virtual void setBacklight(float value) = 0;
    virtual bool exists() = 0;
};

class LedNotification : public NotificationDevice {
  public:
    LedNotification(std::string type);

    void setBacklight(float value);
    bool exists();

  private:
    LedDevice mLed;
};

std::vector<NotificationDevice> getNotificationDevices();

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
