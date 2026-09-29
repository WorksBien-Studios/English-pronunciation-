#include "AudioQuality/AudioQuality.h"
#include "TestFramework.h"

#include <cmath>

using namespace pronunciation;

namespace {

PcmBuffer makeSilence(double seconds, int sampleRateHz = 16000) {
    PcmBuffer buffer;
    buffer.sampleRateHz = sampleRateHz;
    buffer.samples.assign(static_cast<size_t>(seconds * sampleRateHz), 0.0f);
    return buffer;
}

PcmBuffer makeTone(double seconds, float amplitude, float frequencyHz = 220.0f, int sampleRateHz = 16000) {
    PcmBuffer buffer;
    buffer.sampleRateHz = sampleRateHz;
    size_t n = static_cast<size_t>(seconds * sampleRateHz);
    buffer.samples.resize(n);
    for (size_t i = 0; i < n; ++i) {
        buffer.samples[i] = amplitude * std::sin(2.0 * M_PI * frequencyHz * i / sampleRateHz);
    }
    return buffer;
}

// A constant-amplitude tone has near-identical frame energy everywhere, so
// it looks like 0 dB SNR to the noise-floor-vs-signal estimator (there is
// no quiet stretch to establish a floor). Real recordings have a leading
// room-noise floor before speech starts; this helper approximates that so
// the gate sees a realistic SNR profile.
PcmBuffer makeSpeechLikeTone(double seconds, float voicedAmplitude, int sampleRateHz = 16000) {
    PcmBuffer buffer = makeTone(seconds, voicedAmplitude, 220.0f, sampleRateHz);
    size_t noiseFloorSamples = buffer.samples.size() / 5;
    for (size_t i = 0; i < noiseFloorSamples; ++i) buffer.samples[i] *= 0.02f;
    return buffer;
}

} // namespace

TEST(audio_quality_rejects_silence) {
    AudioQualityGate gate;
    auto result = gate.evaluate(makeSilence(1.0));
    REQUIRE(!result.passesGate);
    REQUIRE(result.reason == AudioQualityReason::Silence);
}

TEST(audio_quality_rejects_too_short) {
    AudioQualityGate gate;
    auto result = gate.evaluate(makeTone(0.1, 0.5f));
    REQUIRE(!result.passesGate);
    REQUIRE(result.reason == AudioQualityReason::TooShort);
}

TEST(audio_quality_rejects_clipping) {
    AudioQualityGate gate;
    PcmBuffer clipped = makeTone(1.0, 0.999f);
    for (auto& sample : clipped.samples) sample = sample > 0 ? 1.0f : -1.0f;
    auto result = gate.evaluate(clipped);
    REQUIRE(!result.passesGate);
    REQUIRE(result.reason == AudioQualityReason::Clipping);
}

TEST(audio_quality_accepts_clean_tone) {
    AudioQualityGate gate;
    auto result = gate.evaluate(makeSpeechLikeTone(1.0, 0.5f));
    REQUIRE(result.passesGate);
    REQUIRE(result.reason == AudioQualityReason::Ok);
}
