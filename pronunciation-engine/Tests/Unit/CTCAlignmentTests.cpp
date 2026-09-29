#include "CTCAlignment/CTCAlignment.h"
#include "TestFramework.h"

#include <cmath>

using namespace pronunciation;

namespace {

// Builds log-probabilities for a script where frame t is dominated by
// script[t] with the given confidence, rest of the mass spread uniformly
// (mirrors MockAcousticModel's construction, kept independent here so
// CTCAlignment tests do not depend on the Acoustic module).
FrameLogProbs buildLogProbs(const std::vector<PhonemeId>& script, int vocabSize, int blankColumn, float confidence = 0.9f) {
    FrameLogProbs logProbs;
    logProbs.vocabSize = vocabSize;
    logProbs.blankColumn = blankColumn;
    logProbs.numFrames = static_cast<int>(script.size());
    logProbs.data.assign(static_cast<size_t>(logProbs.numFrames) * vocabSize, 0.0f);
    float remainder = (1.0f - confidence) / static_cast<float>(vocabSize - 1);
    for (int t = 0; t < logProbs.numFrames; ++t) {
        int dominant = script[static_cast<size_t>(t)] == kBlankPhonemeId ? blankColumn : script[static_cast<size_t>(t)];
        for (int v = 0; v < vocabSize; ++v) {
            float p = (v == dominant) ? confidence : remainder;
            logProbs.data[static_cast<size_t>(t) * vocabSize + v] = std::log(p);
        }
    }
    return logProbs;
}

} // namespace

TEST(force_align_recovers_exact_spans) {
    // vocab: {0, 1, 2, blank=3}. Expected sequence [0, 1]. Script: 3 frames
    // of 0, 1 blank frame, 3 frames of 1.
    std::vector<PhonemeId> script = {0, 0, 0, kBlankPhonemeId, 1, 1, 1};
    FrameLogProbs logProbs = buildLogProbs(script, 4, 3);

    auto result = forceAlign(logProbs, {0, 1});
    REQUIRE(result.succeeded);
    REQUIRE(result.phonemes.size() == 2);
    REQUIRE(result.phonemes[0].hasFrames());
    REQUIRE(result.phonemes[0].startFrame == 0);
    REQUIRE(result.phonemes[0].endFrame == 2);
    REQUIRE(result.phonemes[1].hasFrames());
    REQUIRE(result.phonemes[1].startFrame == 4);
    REQUIRE(result.phonemes[1].endFrame == 6);
}

TEST(force_align_handles_adjacent_distinct_labels_without_blank) {
    // No blank needed between two distinct labels. n=2 labels requires at
    // least 2*2+1=5 frames for the constrained alignment graph to fit.
    std::vector<PhonemeId> script = {0, 0, 0, 1, 1};
    FrameLogProbs logProbs = buildLogProbs(script, 4, 3);

    auto result = forceAlign(logProbs, {0, 1});
    REQUIRE(result.succeeded);
    REQUIRE(result.phonemes[0].hasFrames());
    REQUIRE(result.phonemes[1].hasFrames());
}

TEST(force_align_fails_when_audio_too_short_for_expected_sequence) {
    std::vector<PhonemeId> script = {0};
    FrameLogProbs logProbs = buildLogProbs(script, 4, 3);
    // Expected sequence needs at least 2*3+1 = 7 frames for 3 phonemes.
    auto result = forceAlign(logProbs, {0, 1, 2});
    REQUIRE(!result.succeeded);
}

TEST(greedy_decode_collapses_repeats_and_drops_blanks) {
    std::vector<PhonemeId> script = {0, 0, kBlankPhonemeId, 0, 1, 1};
    FrameLogProbs logProbs = buildLogProbs(script, 4, 3, 0.95f);
    auto decoded = greedyDecode(logProbs);
    // 0,0 collapse to one 0; blank resets so the next 0 counts again; 1,1 collapse to one 1.
    std::vector<PhonemeId> expected = {0, 0, 1};
    REQUIRE(decoded == expected);
}

TEST(levenshtein_align_detects_trailing_insertion) {
    std::vector<PhonemeId> hypothesis = {0, 1, 2};
    std::vector<PhonemeId> reference = {0, 1};
    auto ops = levenshteinAlign(hypothesis, reference);
    REQUIRE(!ops.empty());
    REQUIRE(ops.back().type == EditOpType::Insert);
    REQUIRE(ops.back().hypothesisIndex == 2);
}

TEST(levenshtein_align_matches_identical_sequences) {
    std::vector<PhonemeId> sequence = {0, 1, 2};
    auto ops = levenshteinAlign(sequence, sequence);
    for (const auto& op : ops) REQUIRE(op.type == EditOpType::Match);
}
