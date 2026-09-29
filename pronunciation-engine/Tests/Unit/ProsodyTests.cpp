#include "Prosody/Prosody.h"
#include "TestFramework.h"

#include <cmath>

using namespace pronunciation;

namespace {

constexpr double kFrameDurationSeconds = 0.02;
constexpr int kSampleRateHz = 16000;
constexpr int kFrameSamples = static_cast<int>(kFrameDurationSeconds * kSampleRateHz);

PcmBuffer makeTone(int numFrames, float amplitude, float frequencyHz) {
    PcmBuffer buffer;
    buffer.sampleRateHz = kSampleRateHz;
    size_t n = static_cast<size_t>(numFrames) * kFrameSamples;
    buffer.samples.resize(n);
    for (size_t i = 0; i < n; ++i) {
        buffer.samples[i] = amplitude * std::sin(2.0 * M_PI * frequencyHz * i / kSampleRateHz);
    }
    return buffer;
}

} // namespace

TEST(prosody_estimates_pitch_for_vowel_slot) {
    constexpr int numFrames = 10;
    constexpr float f0 = 150.0f;
    PcmBuffer audio = makeTone(numFrames, 0.5f, f0);

    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.isVowel = true;
    exercise.expectedPhonemes.push_back(slot);

    ForcedAlignmentResult alignment;
    alignment.succeeded = true;
    AlignedPhoneme aligned;
    aligned.expectedIndex = 0;
    aligned.startFrame = 0;
    aligned.endFrame = numFrames - 1;
    alignment.phonemes.push_back(aligned);

    ProsodyFeatures features = extractProsody(audio, alignment, exercise, kFrameDurationSeconds);
    REQUIRE(features.pitchHz.size() == 1);
    REQUIRE(features.pitchHz[0] > f0 * 0.9f);
    REQUIRE(features.pitchHz[0] < f0 * 1.1f);
    REQUIRE_NEAR(features.durationsSeconds[0], numFrames * kFrameDurationSeconds, 1e-6);
}

TEST(prosody_skips_pitch_for_non_vowel_slot) {
    constexpr int numFrames = 10;
    PcmBuffer audio = makeTone(numFrames, 0.5f, 150.0f);

    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.isVowel = false;
    exercise.expectedPhonemes.push_back(slot);

    ForcedAlignmentResult alignment;
    alignment.succeeded = true;
    AlignedPhoneme aligned;
    aligned.expectedIndex = 0;
    aligned.startFrame = 0;
    aligned.endFrame = numFrames - 1;
    alignment.phonemes.push_back(aligned);

    ProsodyFeatures features = extractProsody(audio, alignment, exercise, kFrameDurationSeconds);
    REQUIRE(features.pitchHz[0] == 0.0f);
}

TEST(prosody_reports_zero_duration_for_undeleted_span) {
    PcmBuffer audio = makeTone(5, 0.5f, 150.0f);
    ExerciseDefinition exercise;
    exercise.expectedPhonemes.push_back(ExpectedPhonemeSlot{});

    ForcedAlignmentResult alignment;
    alignment.succeeded = true;
    AlignedPhoneme aligned;
    aligned.expectedIndex = 0;
    aligned.startFrame = 2;
    aligned.endFrame = 1; // no frames => a deletion, per AlignedPhoneme::hasFrames()
    alignment.phonemes.push_back(aligned);

    ProsodyFeatures features = extractProsody(audio, alignment, exercise, kFrameDurationSeconds);
    REQUIRE(features.durationsSeconds[0] == 0.0f);
}

// Numeric accuracy checks. Apple builds compute these sums with Accelerate/vDSP in single precision, so these
// bounds are what the Apple CI job holds the vDSP path to; Linux checks the portable double-precision path.
TEST(prosody_energy_and_pitch_match_analytic_values) {
    constexpr int numFrames = 10;
    constexpr float amplitude = 0.5f;
    constexpr float f0 = 200.0f; // an integer 80-sample period at 16 kHz
    PcmBuffer audio = makeTone(numFrames, amplitude, f0);

    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.isVowel = true;
    exercise.expectedPhonemes.push_back(slot);

    ForcedAlignmentResult alignment;
    alignment.succeeded = true;
    AlignedPhoneme aligned;
    aligned.expectedIndex = 0;
    aligned.startFrame = 0;
    aligned.endFrame = numFrames - 1;
    alignment.phonemes.push_back(aligned);

    ProsodyFeatures features = extractProsody(audio, alignment, exercise, kFrameDurationSeconds);

    // A sine's RMS is amplitude / sqrt(2); 0.2 s at 200 Hz is exactly 40 whole cycles.
    const double expectedDb = 20.0 * std::log10(amplitude / std::sqrt(2.0));
    REQUIRE_NEAR(features.energyDb[0], expectedDb, 0.05);
    REQUIRE_NEAR(features.pitchHz[0], f0, 5.0);
}
