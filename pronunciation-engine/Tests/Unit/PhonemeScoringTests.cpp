#include "PhonemeScoring/PhonemeScoring.h"
#include "TestFramework.h"

#include <cmath>

using namespace pronunciation;

namespace {

// Builds a 2-frame FrameLogProbs where every frame repeats the same
// per-column probability distribution, so the GOP average over the span
// equals that distribution exactly (makes the expected math in each test
// trivial to state and verify).
FrameLogProbs makeConstantLogProbs(const std::vector<float>& columnProbabilities, int blankColumn) {
    FrameLogProbs logProbs;
    logProbs.vocabSize = static_cast<int>(columnProbabilities.size());
    logProbs.blankColumn = blankColumn;
    logProbs.numFrames = 2;
    logProbs.data.resize(static_cast<size_t>(logProbs.numFrames) * logProbs.vocabSize);
    for (int t = 0; t < logProbs.numFrames; ++t) {
        for (size_t v = 0; v < columnProbabilities.size(); ++v) {
            logProbs.data[static_cast<size_t>(t) * logProbs.vocabSize + v] = std::log(columnProbabilities[v]);
        }
    }
    return logProbs;
}

ExerciseDefinition makeSingleSlotExercise(PhonemeId expected, std::vector<PhonemeId> confusable, bool isVowel = false) {
    ExerciseDefinition exercise;
    ExpectedPhonemeSlot slot;
    slot.phonemeId = expected;
    slot.confusablePhonemeIds = std::move(confusable);
    slot.isVowel = isVowel;
    slot.errorPatternId = "test-pattern";
    exercise.expectedPhonemes.push_back(slot);
    return exercise;
}

ForcedAlignmentResult makeAlignedSpan(PhonemeId phonemeId, int startFrame, int endFrame) {
    ForcedAlignmentResult alignment;
    alignment.succeeded = true;
    AlignedPhoneme aligned;
    aligned.expectedIndex = 0;
    aligned.phonemeId = phonemeId;
    aligned.startFrame = startFrame;
    aligned.endFrame = endFrame;
    alignment.phonemes.push_back(aligned);
    return alignment;
}

} // namespace

TEST(phoneme_scoring_classifies_clear_target_as_correct) {
    // columns: {target=0, competitor=1, other=2, blank=3}
    FrameLogProbs logProbs = makeConstantLogProbs({0.85f, 0.05f, 0.05f, 0.05f}, 3);
    ExerciseDefinition exercise = makeSingleSlotExercise(0, {1});
    ForcedAlignmentResult alignment = makeAlignedSpan(0, 0, 1);

    auto scores = scorePhonemes(logProbs, alignment, exercise);
    REQUIRE(scores.size() == 1);
    REQUIRE(scores[0].verdict == PhonemeVerdict::Correct);
    REQUIRE(scores[0].gopScore > 0.0f);
}

TEST(phoneme_scoring_classifies_confused_competitor_as_substituted) {
    FrameLogProbs logProbs = makeConstantLogProbs({0.1f, 0.6f, 0.1f, 0.2f}, 3);
    ExerciseDefinition exercise = makeSingleSlotExercise(0, {1});
    ForcedAlignmentResult alignment = makeAlignedSpan(0, 0, 1);

    auto scores = scorePhonemes(logProbs, alignment, exercise);
    REQUIRE(scores[0].verdict == PhonemeVerdict::Substituted);
    REQUIRE(scores[0].competitorPhonemeId == 1);
    REQUIRE(scores[0].gopScore < 0.0f);
}

TEST(phoneme_scoring_classifies_missing_evidence_as_deleted) {
    FrameLogProbs logProbs = makeConstantLogProbs({0.02f, 0.02f, 0.02f, 0.94f}, 3);
    ExerciseDefinition exercise = makeSingleSlotExercise(0, {1});
    ForcedAlignmentResult alignment = makeAlignedSpan(0, 0, 1);

    auto scores = scorePhonemes(logProbs, alignment, exercise);
    REQUIRE(scores[0].verdict == PhonemeVerdict::Deleted);
}

TEST(phoneme_scoring_classifies_empty_span_as_deleted_without_reading_frames) {
    FrameLogProbs logProbs = makeConstantLogProbs({0.85f, 0.05f, 0.05f, 0.05f}, 3);
    ExerciseDefinition exercise = makeSingleSlotExercise(0, {1});
    ForcedAlignmentResult alignment = makeAlignedSpan(0, /*start=*/3, /*end=*/1); // endFrame < startFrame

    auto scores = scorePhonemes(logProbs, alignment, exercise);
    REQUIRE(scores[0].verdict == PhonemeVerdict::Deleted);
    REQUIRE(scores[0].targetPosterior == 0.0f);
}
