/*
 * Copyright (C) 2022 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include "LedDevice.h"
#include "Utils.h"

namespace aidl {
namespace android {
namespace hardware {
namespace light {

class BacklightDevice {
  public:
    virtual ~BacklightDevice() = default;

    virtual void setBacklight(float value) = 0;
    virtual bool exists() = 0;
};

class BacklightBrightness : public BacklightDevice {
  public:
    BacklightBrightness(std::string name);

    void setBacklight(float value);
    bool exists();

  private:
    std::string mBasePath;
    uint32_t mMaxBrightness;

    static const std::string kBacklightBasePath;
    static const uint32_t kDefaultMaxBrightness;
};

class LedBacklight : public BacklightDevice {
  public:
    LedBacklight(std::string type);

    void setBacklight(float value);
    bool exists();

  private:
    LedDevice mLed;
};

std::vector<BacklightDevice> getBacklightDevices();

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
