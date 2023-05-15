/*
 * Copyright (c) 2018,2020-2021, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Changes from Qualcomm Innovation Center are provided under the following license:
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#pragma once

#include <aidl/android/hardware/vibrator/BnVibrator.h>
#include <thread>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

class InputFFDevice {
public:
    InputFFDevice();
    int playEffect(int effectId, EffectStrength es, long *playLengthMs);
    int playPrimitive(int primitiveId, float amplitude, long *playLengthMs);
    int on(int32_t timeoutMs);
    int off();
    int setAmplitude(uint8_t amplitude);
    bool mSupportGain;
    bool mSupportEffects;
    bool mSupportExternalControl;
    bool mInExternalControl;

private:
    int play(int effectId, uint32_t timeoutMs, long *playLengthMs);
    int mVibraFd;
    int16_t mCurrAppId;
    int16_t mCurrMagnitude;
};

class LedVibratorDevice {
public:
    LedVibratorDevice();
    int on(int32_t timeoutMs);
    int off();
    bool mDetected;
private:
    int write_value(const char *file, const char *value);
};

class OffloadGlinkConnection {
public:
    int GlinkOpen(std::string& dev);
    int GlinkClose();
    int GlinkPoll();
    int GlinkRead(uint8_t *data, size_t size);
    int GlinkWrite(uint8_t *buf, size_t buflen);
private:
    std::string dev_name;
    int fd;
};

class PatternOffload {
public:
    PatternOffload();
    void SSREventListener(void);
    void SendPatterns();
    int mEnabled;
private:
    OffloadGlinkConnection GlinkCh;
    int initChannel();
    int sendData(uint8_t *data, int len);
};

class Vibrator : public BnVibrator {
public:
    class InputFFDevice ff;
    class LedVibratorDevice ledVib;
    Vibrator();
    ~Vibrator();
    class PatternOffload Offload;

    ndk::ScopedAStatus getCapabilities(int32_t* _aidl_return) override;
    ndk::ScopedAStatus off() override;
    ndk::ScopedAStatus on(int32_t timeoutMs,
            const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus perform(Effect effect, EffectStrength strength,
            const std::shared_ptr<IVibratorCallback>& callback,
            int32_t* _aidl_return) override;
    ndk::ScopedAStatus getSupportedEffects(std::vector<Effect>* _aidl_return) override;
    ndk::ScopedAStatus setAmplitude(float amplitude) override;
    ndk::ScopedAStatus setExternalControl(bool enabled) override;
    ndk::ScopedAStatus getCompositionDelayMax(int32_t* maxDelayMs);
    ndk::ScopedAStatus getCompositionSizeMax(int32_t* maxSize);
    ndk::ScopedAStatus getSupportedPrimitives(std::vector<CompositePrimitive>* supported) override;
    ndk::ScopedAStatus getPrimitiveDuration(CompositePrimitive primitive,
                                            int32_t* durationMs) override;
    ndk::ScopedAStatus compose(const std::vector<CompositeEffect>& composite,
                               const std::shared_ptr<IVibratorCallback>& callback) override;
    ndk::ScopedAStatus getSupportedAlwaysOnEffects(std::vector<Effect>* _aidl_return) override;
    ndk::ScopedAStatus alwaysOnEnable(int32_t id, Effect effect, EffectStrength strength) override;
    ndk::ScopedAStatus alwaysOnDisable(int32_t id) override;
    ndk::ScopedAStatus getResonantFrequency(float *resonantFreqHz) override;
    ndk::ScopedAStatus getQFactor(float *qFactor) override;
    ndk::ScopedAStatus getFrequencyResolution(float *freqResolutionHz) override;
    ndk::ScopedAStatus getFrequencyMinimum(float *freqMinimumHz) override;
    ndk::ScopedAStatus getBandwidthAmplitudeMap(std::vector<float> *_aidl_return) override;
    ndk::ScopedAStatus getPwlePrimitiveDurationMax(int32_t *durationMs) override;
    ndk::ScopedAStatus getPwleCompositionSizeMax(int32_t *maxSize) override;
    ndk::ScopedAStatus getSupportedBraking(std::vector<Braking>* supported) override;
    ndk::ScopedAStatus composePwle(const std::vector<PrimitivePwle> &composite,
                               const std::shared_ptr<IVibratorCallback> &callback) override;
private:
    static void composePlayThread(Vibrator *vibrator,
                        const std::vector<CompositeEffect>& composite,
                        const std::shared_ptr<IVibratorCallback>& callback);
    std::thread composeThread;
    int epollfd;
    int pipefd[2];
    std::atomic<bool> inComposition;
};

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
