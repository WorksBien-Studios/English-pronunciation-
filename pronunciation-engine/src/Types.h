#pragma once

// Internal C++ types shared across the pronunciation engine modules.
// This header is never exposed across the C boundary (see
// include/PronunciationEngine.h for the narrow C/Swift wrapper surface).

#include <cstdint>
#include <string>
#include <vector>

namespace pronunciation {

// A phoneme is represented internally as an index into a PhonemeInventory
// (see Content/PhonemeInventory.h) rather than as a string, so hot-path
// alignment/scoring code never touches std::string. -1 is reserved for the
// CTC blank symbol.
using PhonemeId = int;
constexpr PhonemeId kBlankPhonemeId = -1;
constexpr PhonemeId kInvalidPhonemeId = -2;

// Normalized mono PCM audio, as produced by the AVAudioEngine capture layer
// on iOS (see docs/product-specification.md#audio-capture). Samples are
// expected to already be resampled to sampleRateHz and scaled to [-1, 1].
struct PcmBuffer {
    std::vector<float> samples;
    int sampleRateHz = 16000;

    double durationSeconds() const {
        return sampleRateHz > 0 ? static_cast<double>(samples.size()) / sampleRateHz : 0.0;
    }
};

// Frame-level acoustic model output: numFrames x vocabSize log-probabilities
// (natural log), row-major. Column `vocabSize - 1` is conventionally the CTC
// blank unit; callers should use blankColumn() rather than assuming a fixed
// index so the acoustic-model backend can be swapped without touching
// downstream code.
struct FrameLogProbs {
    int numFrames = 0;
    int vocabSize = 0;
    int blankColumn = 0;
    double frameDurationSeconds = 0.02;
    std::vector<float> data; // size == numFrames * vocabSize

    float at(int frame, int column) const {
        return data[static_cast<size_t>(frame) * vocabSize + column];
    }
};

// One slot of an exercise's expected phoneme sequence, carrying the
// Japanese-relevant confusion set that GOP-style scoring must restrict
// itself to (docs/product-specification.md: "compare only against the
// Japanese-relevant confusion set for that exercise").
struct ExpectedPhonemeSlot {
    PhonemeId phonemeId = kInvalidPhonemeId;
    std::vector<PhonemeId> confusablePhonemeIds;
    bool isVowel = false;
    bool syllableStressed = false;
    std::string errorPatternId; // empty if this slot has no tracked error pattern
};

// A fully resolved exercise ready for the engine to process: the expected
// phoneme sequence plus enough metadata to drive scoring and prosody
// analysis. Built from the content layer (DiagnosticPrompt / PracticeWord /
// PracticeSentence + JapaneseErrorPattern) by ContentValidation/ContentStore.
struct ExerciseDefinition {
    std::string exerciseId;
    std::vector<ExpectedPhonemeSlot> expectedPhonemes;
};

enum class AudioQualityReason {
    Ok,
    TooShort,
    Silence,
    Clipping,
    LowSignalToNoise,
};

struct AudioQualityResult {
    bool passesGate = false;
    AudioQualityReason reason = AudioQualityReason::Ok;
    float rmsDbfs = -120.0f;
    float peakAmplitude = 0.0f;
    float clippingRatio = 0.0f;
    float estimatedSnrDb = 0.0f;
};

// One expected phoneme mapped onto a (possibly empty) span of acoustic
// frames. An empty span (endFrame < startFrame) means the forced aligner
// could not find supporting evidence for this phoneme, which the phoneme
// scorer treats as a candidate deletion.
struct AlignedPhoneme {
    int expectedIndex = -1;
    PhonemeId phonemeId = kInvalidPhonemeId;
    int startFrame = 0;
    int endFrame = -1;

    bool hasFrames() const { return endFrame >= startFrame; }
};

struct ForcedAlignmentResult {
    bool succeeded = false;
    std::vector<AlignedPhoneme> phonemes;
    double totalLogLikelihood = 0.0;
};

enum class PhonemeVerdict {
    Correct,
    Substituted,
    Inserted,
    Deleted,
    LowConfidence,
};

struct PhonemeScoreResult {
    int expectedIndex = -1;
    PhonemeId expectedPhonemeId = kInvalidPhonemeId;
    PhonemeVerdict verdict = PhonemeVerdict::LowConfidence;
    PhonemeId competitorPhonemeId = kInvalidPhonemeId;
    float targetPosterior = 0.0f;
    float competitorPosterior = 0.0f;
    float gopScore = 0.0f; // log(targetPosterior) - log(competitorPosterior)
};

struct ProsodyFeatures {
    std::vector<float> durationsSeconds;
    std::vector<float> energyDb;
    std::vector<float> pitchHz;         // 0 => unvoiced/undetected for that slot
    std::vector<float> stressProminence; // relative prominence, vowel slots only
    float speakingRateSyllablesPerSecond = 0.0f;
};

enum class ConfidenceLevel {
    Low,
    Medium,
    High,
};

enum class DiagnosisOutcome {
    Pass,          // matched expected pronunciation with acceptable confidence
    Retry,         // evidence insufficient; never guess a specific correction
    SpecificError, // a specific, high-confidence corrective diagnosis
};

struct Diagnosis {
    DiagnosisOutcome outcome = DiagnosisOutcome::Retry;
    ConfidenceLevel confidence = ConfidenceLevel::Low;
    std::string errorPatternId;
    PhonemeId expectedPhonemeId = kInvalidPhonemeId;
    PhonemeId producedPhonemeId = kInvalidPhonemeId;
    int expectedIndex = -1;
    bool errorConfirmedByHistory = false; // true once seen >=2x at high confidence
};

struct EngineResult {
    AudioQualityResult audioQuality;
    ForcedAlignmentResult alignment;
    std::vector<PhonemeScoreResult> phonemeScores;
    ProsodyFeatures prosody;
    Diagnosis diagnosis;
};

} // namespace pronunciation
