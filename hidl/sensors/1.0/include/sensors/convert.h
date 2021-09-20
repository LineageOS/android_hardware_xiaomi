/*
 * Copyright (C) 2016 The Android Open Source Project
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

#pragma once

#include <android/hardware/sensors/1.0/ISensors.h>
#include <hardware/sensors.h>

namespace android {
namespace hardware {
namespace sensors {
namespace V1_0 {
namespace implementation {

void convertFromSensor(const sensor_t& src, SensorInfo* dst);
void convertToSensor(const SensorInfo& src, sensor_t* dst);

void convertFromSensorEvent(const sensors_event_t& src, Event* dst);
void convertToSensorEvent(const Event& src, sensors_event_t* dst);

bool convertFromSharedMemInfo(const SharedMemInfo& memIn, sensors_direct_mem_t* memOut);
int convertFromRateLevel(RateLevel rate);

bool patchXiaomiPickupSensor(SensorInfo& sensor);

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
