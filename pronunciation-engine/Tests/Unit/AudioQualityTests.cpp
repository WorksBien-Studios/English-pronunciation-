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

namespace {

// Deterministic pseudo-random source so the rejection sweep is reproducible.
struct Lcg {
    unsigned state;
    explicit Lcg(unsigned seed) : state(seed) {}
    float next() { // uniform in [-1, 1)
        state = state * 1664525u + 1013904223u;
        return static_cast<float>(state >> 8) / static_cast<float>(1u << 23) - 1.0f;
    }
};

} // namespace

// docs/product-specification.md pre-UI gate: "unusable/silent/clipped audio is
// rejected rather than scored at least 95% of the time." Synthetic evidence
// only; real-device recordings are checked separately by hand.
TEST(audio_quality_rejects_at_least_95_percent_of_unusable_audio) {
    AudioQualityGate gate;
    Lcg rng(12345u);
    int total = 0;
    int rejected = 0;
    auto count = [&](const PcmBuffer& audio) {
        ++total;
        if (!gate.evaluate(audio).passesGate) ++rejected;
    };

    for (int i = 0; i < 50; ++i) { // silence and near-silence, up to about -52 dBFS RMS
        PcmBuffer audio = makeSilence(0.5 + 0.02 * i);
        float level = 0.0025f * (i / 50.0f);
        for (auto& s : audio.samples) s = level * rng.next();
        count(audio);
    }
    for (int i = 0; i < 50; ++i) { // heavily clipped speech-like signal (at least 3% of samples pinned)
        PcmBuffer audio = makeSpeechLikeTone(0.6 + 0.02 * i, 1.6f + 0.02f * i);
        for (auto& s : audio.samples) s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s);
        count(audio);
    }
    for (int i = 0; i < 50; ++i) { // shorter than the minimum duration
        count(makeTone(0.02 + 0.005 * i, 0.5f));
    }
    for (int i = 0; i < 50; ++i) { // noise as loud as the signal (about 0 to 5 dB SNR)
        PcmBuffer audio = makeTone(0.8 + 0.01 * i, 0.3f);
        float noise = 0.25f + 0.002f * i;
        for (auto& s : audio.samples) s += noise * rng.next();
        count(audio);
    }

    REQUIRE(total == 200);
    REQUIRE(rejected * 100 >= total * 95);
}

TEST(audio_quality_accepts_clean_speech_like_audio_across_levels) {
    AudioQualityGate gate;
    for (int i = 0; i < 20; ++i) {
        float amplitude = 0.05f + 0.03f * i; // 0.05 to 0.62, well below the clipping limit
        auto result = gate.evaluate(makeSpeechLikeTone(0.8 + 0.05 * i, amplitude));
        REQUIRE(result.passesGate);
    }
}
