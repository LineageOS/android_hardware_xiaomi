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

#include "convert.h"

#include <android-base/logging.h>

namespace android {
namespace hardware {
namespace sensors {
namespace V1_0 {
namespace implementation {

void convertFromSensor(const sensor_t& src, SensorInfo* dst) {
    dst->name = src.name;
    dst->vendor = src.vendor;
    dst->version = src.version;
    dst->sensorHandle = src.handle;
    dst->type = (SensorType)src.type;
    dst->maxRange = src.maxRange;
    dst->resolution = src.resolution;
    dst->power = src.power;
    dst->minDelay = src.minDelay;
    dst->fifoReservedEventCount = src.fifoReservedEventCount;
    dst->fifoMaxEventCount = src.fifoMaxEventCount;
    dst->typeAsString = src.stringType;
    dst->requiredPermission = src.requiredPermission;
    dst->maxDelay = src.maxDelay;
    dst->flags = src.flags;
}

void convertToSensor(const ::android::hardware::sensors::V1_0::SensorInfo& src, sensor_t* dst) {
    dst->name = strdup(src.name.c_str());
    dst->vendor = strdup(src.vendor.c_str());
    dst->version = src.version;
    dst->handle = src.sensorHandle;
    dst->type = (int)src.type;
    dst->maxRange = src.maxRange;
    dst->resolution = src.resolution;
    dst->power = src.power;
    dst->minDelay = src.minDelay;
    dst->fifoReservedEventCount = src.fifoReservedEventCount;
    dst->fifoMaxEventCount = src.fifoMaxEventCount;
    dst->stringType = strdup(src.typeAsString.c_str());
    dst->requiredPermission = strdup(src.requiredPermission.c_str());
    dst->maxDelay = src.maxDelay;
    dst->flags = src.flags;
    dst->reserved[0] = dst->reserved[1] = 0;
}

void convertFromSensorEvent(const sensors_event_t& src, Event* dst) {
    typedef ::android::hardware::sensors::V1_0::SensorType SensorType;
    typedef ::android::hardware::sensors::V1_0::MetaDataEventType MetaDataEventType;

    *dst = {
            .timestamp = src.timestamp,
            .sensorHandle = src.sensor,
            .sensorType = (SensorType)src.type,
    };

    switch (dst->sensorType) {
        case SensorType::META_DATA: {
            dst->u.meta.what = (MetaDataEventType)src.meta_data.what;
            // Legacy HALs contain the handle reference in the meta data field.
            // Copy that over to the handle of the event. In legacy HALs this
            // field was expected to be 0.
            dst->sensorHandle = src.meta_data.sensor;
            break;
        }

        case SensorType::ACCELEROMETER:
        case SensorType::MAGNETIC_FIELD:
        case SensorType::ORIENTATION:
        case SensorType::GYROSCOPE:
        case SensorType::GRAVITY:
        case SensorType::LINEAR_ACCELERATION: {
            dst->u.vec3.x = src.acceleration.x;
            dst->u.vec3.y = src.acceleration.y;
            dst->u.vec3.z = src.acceleration.z;
            dst->u.vec3.status = (SensorStatus)src.acceleration.status;
            break;
        }

        case SensorType::GAME_ROTATION_VECTOR: {
            dst->u.vec4.x = src.data[0];
            dst->u.vec4.y = src.data[1];
            dst->u.vec4.z = src.data[2];
            dst->u.vec4.w = src.data[3];
            break;
        }

        case SensorType::ROTATION_VECTOR:
        case SensorType::GEOMAGNETIC_ROTATION_VECTOR: {
            dst->u.data[0] = src.data[0];
            dst->u.data[1] = src.data[1];
            dst->u.data[2] = src.data[2];
            dst->u.data[3] = src.data[3];
            dst->u.data[4] = src.data[4];
            break;
        }

        case SensorType::MAGNETIC_FIELD_UNCALIBRATED:
        case SensorType::GYROSCOPE_UNCALIBRATED:
        case SensorType::ACCELEROMETER_UNCALIBRATED: {
            dst->u.uncal.x = src.uncalibrated_gyro.x_uncalib;
            dst->u.uncal.y = src.uncalibrated_gyro.y_uncalib;
            dst->u.uncal.z = src.uncalibrated_gyro.z_uncalib;
            dst->u.uncal.x_bias = src.uncalibrated_gyro.x_bias;
            dst->u.uncal.y_bias = src.uncalibrated_gyro.y_bias;
            dst->u.uncal.z_bias = src.uncalibrated_gyro.z_bias;
            break;
        }

        case SensorType::DEVICE_ORIENTATION:
        case SensorType::LIGHT:
        case SensorType::PRESSURE:
        case SensorType::TEMPERATURE:
        case SensorType::PROXIMITY:
        case SensorType::RELATIVE_HUMIDITY:
        case SensorType::AMBIENT_TEMPERATURE:
        case SensorType::SIGNIFICANT_MOTION:
        case SensorType::STEP_DETECTOR:
        case SensorType::TILT_DETECTOR:
        case SensorType::WAKE_GESTURE:
        case SensorType::GLANCE_GESTURE:
        case SensorType::PICK_UP_GESTURE:
        case SensorType::WRIST_TILT_GESTURE:
        case SensorType::STATIONARY_DETECT:
        case SensorType::MOTION_DETECT:
        case SensorType::HEART_BEAT:
        case SensorType::LOW_LATENCY_OFFBODY_DETECT: {
            dst->u.scalar = src.data[0];
            break;
        }

        case SensorType::STEP_COUNTER: {
            dst->u.stepCount = src.u64.step_counter;
            break;
        }

        case SensorType::HEART_RATE: {
            dst->u.heartRate.bpm = src.heart_rate.bpm;
            dst->u.heartRate.status = (SensorStatus)src.heart_rate.status;
            break;
        }

        case SensorType::POSE_6DOF: {  // 15 floats
            for (size_t i = 0; i < 15; ++i) {
                dst->u.pose6DOF[i] = src.data[i];
            }
            break;
        }

        case SensorType::DYNAMIC_SENSOR_META: {
            dst->u.dynamic.connected = src.dynamic_sensor_meta.connected;
            dst->u.dynamic.sensorHandle = src.dynamic_sensor_meta.handle;

            memcpy(dst->u.dynamic.uuid.data(), src.dynamic_sensor_meta.uuid, 16);

            break;
        }

        case SensorType::ADDITIONAL_INFO: {
            ::android::hardware::sensors::V1_0::AdditionalInfo* dstInfo = &dst->u.additional;

            const additional_info_event_t& srcInfo = src.additional_info;

            dstInfo->type = (::android::hardware::sensors::V1_0::AdditionalInfoType)srcInfo.type;

            dstInfo->serial = srcInfo.serial;

            CHECK_EQ(sizeof(dstInfo->u), sizeof(srcInfo.data_int32));
            memcpy(&dstInfo->u, srcInfo.data_int32, sizeof(srcInfo.data_int32));
            break;
        }

        default: {
            memcpy(dst->u.data.data(), src.data, 16 * sizeof(float));
            break;
        }
    }
}

void convertToSensorEvent(const Event& src, sensors_event_t* dst) {
    *dst = {.version = sizeof(sensors_event_t),
            .sensor = src.sensorHandle,
            .type = (int32_t)src.sensorType,
            .reserved0 = 0,
            .timestamp = src.timestamp};

    switch (src.sensorType) {
        case SensorType::META_DATA: {
            // Legacy HALs expect the handle reference in the meta data field.
            // Copy it over from the handle of the event.
            dst->meta_data.what = (int32_t)src.u.meta.what;
            dst->meta_data.sensor = src.sensorHandle;
            // Set the sensor handle to 0 to maintain compatibility.
            dst->sensor = 0;
            break;
        }

        case SensorType::ACCELEROMETER:
        case SensorType::MAGNETIC_FIELD:
        case SensorType::ORIENTATION:
        case SensorType::GYROSCOPE:
        case SensorType::GRAVITY:
        case SensorType::LINEAR_ACCELERATION: {
            dst->acceleration.x = src.u.vec3.x;
            dst->acceleration.y = src.u.vec3.y;
            dst->acceleration.z = src.u.vec3.z;
            dst->acceleration.status = (int8_t)src.u.vec3.status;
            break;
        }

        case SensorType::GAME_ROTATION_VECTOR: {
            dst->data[0] = src.u.vec4.x;
            dst->data[1] = src.u.vec4.y;
            dst->data[2] = src.u.vec4.z;
            dst->data[3] = src.u.vec4.w;
            break;
        }

        case SensorType::ROTATION_VECTOR:
        case SensorType::GEOMAGNETIC_ROTATION_VECTOR: {
            dst->data[0] = src.u.data[0];
            dst->data[1] = src.u.data[1];
            dst->data[2] = src.u.data[2];
            dst->data[3] = src.u.data[3];
            dst->data[4] = src.u.data[4];
            break;
        }

        case SensorType::MAGNETIC_FIELD_UNCALIBRATED:
        case SensorType::GYROSCOPE_UNCALIBRATED:
        case SensorType::ACCELEROMETER_UNCALIBRATED: {
            dst->uncalibrated_gyro.x_uncalib = src.u.uncal.x;
            dst->uncalibrated_gyro.y_uncalib = src.u.uncal.y;
            dst->uncalibrated_gyro.z_uncalib = src.u.uncal.z;
            dst->uncalibrated_gyro.x_bias = src.u.uncal.x_bias;
            dst->uncalibrated_gyro.y_bias = src.u.uncal.y_bias;
            dst->uncalibrated_gyro.z_bias = src.u.uncal.z_bias;
            break;
        }

        case SensorType::DEVICE_ORIENTATION:
        case SensorType::LIGHT:
        case SensorType::PRESSURE:
        case SensorType::TEMPERATURE:
        case SensorType::PROXIMITY:
        case SensorType::RELATIVE_HUMIDITY:
        case SensorType::AMBIENT_TEMPERATURE:
        case SensorType::SIGNIFICANT_MOTION:
        case SensorType::STEP_DETECTOR:
        case SensorType::TILT_DETECTOR:
        case SensorType::WAKE_GESTURE:
        case SensorType::GLANCE_GESTURE:
        case SensorType::PICK_UP_GESTURE:
        case SensorType::WRIST_TILT_GESTURE:
        case SensorType::STATIONARY_DETECT:
        case SensorType::MOTION_DETECT:
        case SensorType::HEART_BEAT:
        case SensorType::LOW_LATENCY_OFFBODY_DETECT: {
            dst->data[0] = src.u.scalar;
            break;
        }

        case SensorType::STEP_COUNTER: {
            dst->u64.step_counter = src.u.stepCount;
            break;
        }

        case SensorType::HEART_RATE: {
            dst->heart_rate.bpm = src.u.heartRate.bpm;
            dst->heart_rate.status = (int8_t)src.u.heartRate.status;
            break;
        }

        case SensorType::POSE_6DOF: {  // 15 floats
            for (size_t i = 0; i < 15; ++i) {
                dst->data[i] = src.u.pose6DOF[i];
            }
            break;
        }

        case SensorType::DYNAMIC_SENSOR_META: {
            dst->dynamic_sensor_meta.connected = src.u.dynamic.connected;
            dst->dynamic_sensor_meta.handle = src.u.dynamic.sensorHandle;
            dst->dynamic_sensor_meta.sensor = NULL;  // to be filled in later

            memcpy(dst->dynamic_sensor_meta.uuid, src.u.dynamic.uuid.data(), 16);

            break;
        }

        case SensorType::ADDITIONAL_INFO: {
            const ::android::hardware::sensors::V1_0::AdditionalInfo& srcInfo = src.u.additional;

            additional_info_event_t* dstInfo = &dst->additional_info;
            dstInfo->type = (int32_t)srcInfo.type;
            dstInfo->serial = srcInfo.serial;

            CHECK_EQ(sizeof(srcInfo.u), sizeof(dstInfo->data_int32));

            memcpy(dstInfo->data_int32, &srcInfo.u, sizeof(dstInfo->data_int32));

            break;
        }

        default: {
            memcpy(dst->data, src.u.data.data(), 16 * sizeof(float));
            break;
        }
    }
}

bool convertFromSharedMemInfo(const SharedMemInfo& memIn, sensors_direct_mem_t* memOut) {
    if (memOut == nullptr) {
        return false;
    }

    switch (memIn.type) {
        case SharedMemType::ASHMEM:
            memOut->type = SENSOR_DIRECT_MEM_TYPE_ASHMEM;
            break;
        case SharedMemType::GRALLOC:
            memOut->type = SENSOR_DIRECT_MEM_TYPE_GRALLOC;
            break;
        default:
            return false;
    }

    switch (memIn.format) {
        case SharedMemFormat::SENSORS_EVENT:
            memOut->format = SENSOR_DIRECT_FMT_SENSORS_EVENT;
            break;
        default:
            return false;
    }

    if (memIn.memoryHandle == nullptr) {
        return false;
    }

    memOut->size = memIn.size;
    memOut->handle = memIn.memoryHandle;
    return true;
}

int convertFromRateLevel(RateLevel rate) {
    switch (rate) {
        case RateLevel::STOP:
            return SENSOR_DIRECT_RATE_STOP;
        case RateLevel::NORMAL:
            return SENSOR_DIRECT_RATE_NORMAL;
        case RateLevel::FAST:
            return SENSOR_DIRECT_RATE_FAST;
        case RateLevel::VERY_FAST:
            return SENSOR_DIRECT_RATE_VERY_FAST;
        default:
            return -1;
    }
}

bool patchXiaomiPickupSensor(SensorInfo& sensor) {
    if (sensor.typeAsString != "xiaomi.sensor.pickup" &&
        sensor.typeAsString != "xiaomi pick up sensor") {
        return true;
    }

    /*
     * Implement only the wake-up version of this sensor.
     */
    if (!(sensor.flags & SensorFlagBits::WAKE_UP)) {
        return false;
    }

    sensor.type = SensorType::PICK_UP_GESTURE;
    sensor.typeAsString = SENSOR_STRING_TYPE_PICK_UP_GESTURE;
    sensor.maxRange = 1;

    return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensors
}  // namespace hardware
}  // namespace android
