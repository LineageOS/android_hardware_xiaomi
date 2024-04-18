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

class ButtonDevice {
  public:
    virtual ~ButtonDevice() = default;

    virtual void setBacklight(float value) = 0;
    virtual bool exists() = 0;
};

class LedButton : public ButtonDevice {
  public:
    LedButton(std::string type);

    void setBacklight(float value);
    bool exists();

  private:
    LedDevice mLed;
};

std::vector<ButtonDevice> getButtonDevices();

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
