#include "Acoustic/MockAcousticModel.h"
#include "AudioQuality/AudioQuality.h"
#include "Content/ContentStore.h"
#include "Content/PhonemeInventory.h"
#include "DecisionRules/DecisionRules.h"
#include "Engine.h"
#include "TestFramework.h"

#include <cmath>
#include <limits>
#include <memory>
#include <string>

using namespace pronunciation;

namespace {

const std::string kResourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;

PcmBuffer makeSpeechLikeTone(double seconds, int sampleRateHz = 16000) {
    PcmBuffer buffer;
    buffer.sampleRateHz = sampleRateHz;
    size_t n = static_cast<size_t>(seconds * sampleRateHz);
    buffer.samples.resize(n);
    for (size_t i = 0; i < n; ++i) {
        buffer.samples[i] = 0.5f * std::sin(2.0 * M_PI * 220.0 * i / sampleRateHz);
    }
    size_t quietSamples = n / 5;
    for (size_t i = 0; i < quietSamples; ++i) buffer.samples[i] *= 0.02f;
    return buffer;
}

ContentStore loadContent(PhonemeInventory& inventory) {
    inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    return ContentStore::loadFromDirectory(kResourcesDir, inventory);
}

} // namespace

TEST(stress_low_confidence_phoneme_never_becomes_high_confidence_pass) {
    PhonemeInventory inventory;
    ContentStore content = loadContent(inventory);
    ErrorHistory history;

    AudioQualityResult audio;
    audio.passesGate = true;

    ExpectedPhonemeSlot slot;
    slot.phonemeId = inventory.idForSymbol("R");
    slot.errorPatternId = "rl-substitution";
    slot.confusablePhonemeIds = {inventory.idForSymbol("L")};

    ExerciseDefinition exercise;
    exercise.expectedPhonemes = {slot};

    PhonemeScoreResult score;
    score.expectedIndex = 0;
    score.expectedPhonemeId = slot.phonemeId;
    score.verdict = PhonemeVerdict::LowConfidence;
    score.targetPosterior = 0.40f;
    score.competitorPosterior = 0.35f;

    Diagnosis diagnosis = decideDiagnosis(audio, {score}, exercise, content, {}, history);
    REQUIRE(diagnosis.outcome == DiagnosisOutcome::Retry);
    REQUIRE(diagnosis.confidence == ConfidenceLevel::Low);
}

TEST(stress_untracked_severe_substitution_never_silently_passes) {
    PhonemeInventory inventory;
    ContentStore content = loadContent(inventory);
    ErrorHistory history;

    AudioQualityResult audio;
    audio.passesGate = true;

    ExpectedPhonemeSlot slot;
    slot.phonemeId = inventory.idForSymbol("K");
    // No Japanese-specific error pattern is attached to this slot.

    ExerciseDefinition exercise;
    exercise.expectedPhonemes = {slot};

    PhonemeScoreResult score;
    score.expectedIndex = 0;
    score.expectedPhonemeId = slot.phonemeId;
    score.verdict = PhonemeVerdict::Substituted;
    score.competitorPhonemeId = inventory.idForSymbol("G");
    score.targetPosterior = 0.02f;
    score.competitorPosterior = 0.95f;

    Diagnosis diagnosis = decideDiagnosis(audio, {score}, exercise, content, {}, history);
    REQUIRE(diagnosis.outcome == DiagnosisOutcome::Retry);
    REQUIRE(diagnosis.confidence == ConfidenceLevel::Low);
}

TEST(stress_audio_quality_rejects_nan_samples) {
    AudioQualityGate gate;
    PcmBuffer audio = makeSpeechLikeTone(1.0);
    audio.samples[audio.samples.size() / 2] = std::numeric_limits<float>::quiet_NaN();

    AudioQualityResult result = gate.evaluate(audio);
    REQUIRE(!result.passesGate);
}

TEST(stress_audio_quality_rejects_infinite_samples) {
    AudioQualityGate gate;
    PcmBuffer audio = makeSpeechLikeTone(1.0);
    audio.samples[audio.samples.size() / 2] = std::numeric_limits<float>::infinity();

    AudioQualityResult result = gate.evaluate(audio);
    REQUIRE(!result.passesGate);
}

TEST(stress_engine_detects_internal_vowel_epenthesis) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    int vocabSize = static_cast<int>(inventory.size()) + 1;
    int blank = static_cast<int>(inventory.size());

    auto model = std::make_unique<MockAcousticModel>(vocabSize, blank);
    // "cat" is K AE T. Insert AX after K, i.e. an internal Japanese-style
    // epenthetic vowel rather than only a trailing vowel after the word.
    model->setScriptFromSequence({
        {inventory.idForSymbol("K"), 0.95f},
        {inventory.idForSymbol("AX"), 0.95f},
        {inventory.idForSymbol("AE"), 0.95f},
        {inventory.idForSymbol("T"), 0.95f},
    }, 5);

    PronunciationEngine engine(kResourcesDir, std::move(model));
    EngineResult result = engine.process(makeSpeechLikeTone(1.0), "word:cat");

    REQUIRE(result.audioQuality.passesGate);
    REQUIRE(result.diagnosis.outcome == DiagnosisOutcome::SpecificError);
    REQUIRE(result.diagnosis.errorPatternId == "vowel-insertion-after-consonant");
}

TEST(stress_nonfinite_mock_posterior_never_produces_pass) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    int vocabSize = static_cast<int>(inventory.size()) + 1;
    int blank = static_cast<int>(inventory.size());

    auto model = std::make_unique<MockAcousticModel>(vocabSize, blank);
    const float nan = std::numeric_limits<float>::quiet_NaN();
    model->setScriptFromSequence({
        {inventory.idForSymbol("K"), nan},
        {inventory.idForSymbol("AE"), nan},
        {inventory.idForSymbol("T"), nan},
    }, 5);

    PronunciationEngine engine(kResourcesDir, std::move(model));
    EngineResult result = engine.process(makeSpeechLikeTone(1.0), "word:cat");
    REQUIRE(result.diagnosis.outcome != DiagnosisOutcome::Pass);
}
