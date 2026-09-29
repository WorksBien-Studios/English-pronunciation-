#include "Engine.h"
#include "Acoustic/MockAcousticModel.h"
#include "TestFramework.h"

#include <cmath>
#include <memory>

using namespace pronunciation;

namespace {
const std::string kResourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;

PcmBuffer makeSpeechLikeTone(double seconds, int sampleRateHz = 16000) {
    PcmBuffer buffer;
    buffer.sampleRateHz = sampleRateHz;
    size_t n = static_cast<size_t>(seconds * sampleRateHz);
    buffer.samples.resize(n);
    for (size_t i = 0; i < n; ++i) buffer.samples[i] = 0.5f * std::sin(2.0 * M_PI * 220.0 * i / sampleRateHz);
    size_t noiseFloorSamples = n / 5;
    for (size_t i = 0; i < noiseFloorSamples; ++i) buffer.samples[i] *= 0.02f;
    return buffer;
}

std::pair<int, int> inventoryVocab(const PhonemeInventory& inventory) {
    return {static_cast<int>(inventory.size()) + 1, static_cast<int>(inventory.size())};
}

} // namespace

TEST(engine_passes_a_correctly_pronounced_word) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    auto [vocabSize, blank] = inventoryVocab(inventory);

    auto model = std::make_unique<MockAcousticModel>(vocabSize, blank);
    model->setScriptFromSequence({{inventory.idForSymbol("K"), 0.9f}, {inventory.idForSymbol("AE"), 0.9f}, {inventory.idForSymbol("T"), 0.9f}},
                                  5);

    PronunciationEngine engine(kResourcesDir, std::move(model));
    EngineResult result = engine.process(makeSpeechLikeTone(1.0), "word:cat");

    REQUIRE(result.audioQuality.passesGate);
    REQUIRE(result.diagnosis.outcome == DiagnosisOutcome::Pass);
}

TEST(engine_flags_rl_substitution_and_confirms_after_two_attempts) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    auto [vocabSize, blank] = inventoryVocab(inventory);

    auto model = std::make_unique<MockAcousticModel>(vocabSize, blank);
    // "right" is R AY T; script L instead of R. The engine keeps this model
    // (and its ErrorHistory) alive across repeated process() calls, so two
    // attempts on the same engine is what exercises the repeats-required-
    // to-confirm rule end to end.
    model->setScriptFromSequence(
        {{inventory.idForSymbol("L"), 0.9f}, {inventory.idForSymbol("AY"), 0.9f}, {inventory.idForSymbol("T"), 0.9f}}, 5);

    PronunciationEngine engine(kResourcesDir, std::move(model));

    EngineResult first = engine.process(makeSpeechLikeTone(1.0), "word:right");
    REQUIRE(first.diagnosis.outcome == DiagnosisOutcome::SpecificError);
    REQUIRE(first.diagnosis.errorPatternId == "rl-substitution");
    REQUIRE(!first.diagnosis.errorConfirmedByHistory);

    EngineResult second = engine.process(makeSpeechLikeTone(1.0), "word:right");
    REQUIRE(second.diagnosis.outcome == DiagnosisOutcome::SpecificError);
    REQUIRE(second.diagnosis.errorConfirmedByHistory);
}

TEST(engine_retries_on_silent_audio_without_running_the_acoustic_model) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    auto [vocabSize, blank] = inventoryVocab(inventory);
    auto model = std::make_unique<MockAcousticModel>(vocabSize, blank);
    model->setScriptFromSequence({{inventory.idForSymbol("K"), 0.9f}}, 5);

    PronunciationEngine engine(kResourcesDir, std::move(model));
    PcmBuffer silence;
    silence.sampleRateHz = 16000;
    silence.samples.assign(16000, 0.0f);

    EngineResult result = engine.process(silence, "word:cat");
    REQUIRE(!result.audioQuality.passesGate);
    REQUIRE(result.diagnosis.outcome == DiagnosisOutcome::Retry);
    REQUIRE(result.phonemeScores.empty());
}
