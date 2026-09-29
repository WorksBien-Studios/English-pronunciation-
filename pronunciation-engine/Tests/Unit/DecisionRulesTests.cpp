#include "DecisionRules/DecisionRules.h"
#include "TestFramework.h"

using namespace pronunciation;

namespace {

ContentStore loadTestContent(PhonemeInventory& inventoryOut) {
    inventoryOut = PhonemeInventory::loadFromFile(std::string(PRONUNCIATION_ENGINE_RESOURCES_DIR) + "/phonemes.json");
    return ContentStore::loadFromDirectory(PRONUNCIATION_ENGINE_RESOURCES_DIR, inventoryOut);
}

PhonemeScoreResult makeSubstitution(int expectedIndex, PhonemeId expected, PhonemeId competitor, float competitorPosterior) {
    PhonemeScoreResult score;
    score.expectedIndex = expectedIndex;
    score.expectedPhonemeId = expected;
    score.verdict = PhonemeVerdict::Substituted;
    score.competitorPhonemeId = competitor;
    score.competitorPosterior = competitorPosterior;
    score.targetPosterior = 1.0f - competitorPosterior;
    return score;
}

} // namespace

TEST(decision_rules_retries_immediately_on_bad_audio_quality) {
    PhonemeInventory inventory;
    ContentStore content = loadTestContent(inventory);
    ErrorHistory history;

    AudioQualityResult badAudio;
    badAudio.passesGate = false;
    badAudio.reason = AudioQualityReason::Silence;

    Diagnosis diagnosis = decideDiagnosis(badAudio, {}, {}, content, {}, history);
    REQUIRE(diagnosis.outcome == DiagnosisOutcome::Retry);
    REQUIRE(diagnosis.confidence == ConfidenceLevel::Low);
}

TEST(decision_rules_passes_when_no_error_candidates) {
    PhonemeInventory inventory;
    ContentStore content = loadTestContent(inventory);
    ErrorHistory history;

    AudioQualityResult goodAudio;
    goodAudio.passesGate = true;

    PhonemeScoreResult correct;
    correct.expectedIndex = 0;
    correct.verdict = PhonemeVerdict::Correct;

    ExerciseDefinition exercise;
    exercise.expectedPhonemes.push_back(ExpectedPhonemeSlot{});

    Diagnosis diagnosis = decideDiagnosis(goodAudio, {correct}, exercise, content, {}, history);
    REQUIRE(diagnosis.outcome == DiagnosisOutcome::Pass);
}

TEST(decision_rules_confirms_error_only_after_two_high_confidence_observations) {
    PhonemeInventory inventory;
    ContentStore content = loadTestContent(inventory);
    ErrorHistory history;

    const JapaneseErrorPattern* pattern = content.findErrorPattern("rl-substitution");
    REQUIRE(pattern != nullptr);
    PhonemeId rId = inventory.idForSymbol("R");
    PhonemeId lId = inventory.idForSymbol("L");

    AudioQualityResult goodAudio;
    goodAudio.passesGate = true;

    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.phonemeId = rId;
    slot.confusablePhonemeIds = {lId};
    slot.errorPatternId = "rl-substitution";
    exercise.expectedPhonemes.push_back(slot);

    // Evidence clears the pattern's locked confidence threshold (0.62).
    auto score = makeSubstitution(0, rId, lId, pattern->confidenceThreshold + 0.1f);

    Diagnosis first = decideDiagnosis(goodAudio, {score}, exercise, content, {}, history);
    REQUIRE(first.outcome == DiagnosisOutcome::SpecificError);
    REQUIRE(first.errorPatternId == "rl-substitution");
    REQUIRE(!first.errorConfirmedByHistory);

    Diagnosis second = decideDiagnosis(goodAudio, {score}, exercise, content, {}, history);
    REQUIRE(second.outcome == DiagnosisOutcome::SpecificError);
    REQUIRE(second.errorConfirmedByHistory);
}

TEST(decision_rules_never_diagnoses_below_locked_confidence_threshold) {
    PhonemeInventory inventory;
    ContentStore content = loadTestContent(inventory);
    ErrorHistory history;

    const JapaneseErrorPattern* pattern = content.findErrorPattern("rl-substitution");
    PhonemeId rId = inventory.idForSymbol("R");
    PhonemeId lId = inventory.idForSymbol("L");

    AudioQualityResult goodAudio;
    goodAudio.passesGate = true;

    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.phonemeId = rId;
    slot.confusablePhonemeIds = {lId};
    slot.errorPatternId = "rl-substitution";
    exercise.expectedPhonemes.push_back(slot);

    // Evidence exists but never clears the locked threshold.
    auto score = makeSubstitution(0, rId, lId, pattern->confidenceThreshold - 0.1f);

    Diagnosis diagnosis = decideDiagnosis(goodAudio, {score}, exercise, content, {}, history);
    REQUIRE(diagnosis.outcome == DiagnosisOutcome::Retry);
}
