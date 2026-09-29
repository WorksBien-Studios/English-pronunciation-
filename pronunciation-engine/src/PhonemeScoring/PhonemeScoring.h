#pragma once

#include <vector>

#include "../Types.h"

namespace pronunciation {

struct PhonemeScoringThresholds {
    float deletionPosteriorFloor = 0.05f;    // average target posterior below this => Deleted
    float correctPosteriorFloor = 0.5f;      // average target posterior at/above this (and winning) => Correct
    float substitutionPosteriorFloor = 0.35f; // competitor must clear this to call a Substitution
};

// Pipeline steps 5-6 (docs/product-specification.md#model-strategy):
// "calculate per-phoneme expected-versus-competing posterior scores using a
// GOP-style method" restricted to "only the Japanese-relevant confusion set
// for that exercise". For each aligned phoneme, this averages the acoustic
// model's per-frame log-probability of the expected phoneme over its
// aligned span (the standard Goodness-Of-Pronunciation summary statistic)
// and compares it against the best-scoring phoneme in that slot's
// confusable set (falling back to the best-scoring phoneme overall when the
// content layer did not attach a confusion set, e.g. a slot with no
// tracked Japanese error pattern).
//
// This function only measures evidence; it does not decide pass/retry/
// specific-error — that conservative judgement, including confidence
// thresholds and repeated-error history, belongs to DecisionRules.
std::vector<PhonemeScoreResult> scorePhonemes(const FrameLogProbs& logProbs,
                                               const ForcedAlignmentResult& alignment,
                                               const ExerciseDefinition& exercise,
                                               const PhonemeScoringThresholds& thresholds = {});

} // namespace pronunciation
