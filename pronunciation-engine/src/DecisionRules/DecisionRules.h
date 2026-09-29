#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../Content/ContentStore.h"
#include "../Types.h"
#include "ErrorHistory.h"

namespace pronunciation {

// An exercise-level pattern candidate detected outside the per-slot GOP
// scores (e.g. an epenthetic vowel found by comparing the free CTC decode
// against the expected sequence). See Engine.cpp for how this is produced.
struct GlobalPatternCandidate {
    std::string errorPatternId;
    float evidenceScore = 0.0f; // 0..1, compared against the pattern's confidenceThreshold
};

// Pipeline steps 9-10 (docs/product-specification.md#model-strategy):
// "fuse the signals conservatively and return a typed confidence level" and
// "map only high-confidence repeated errors to deterministic Japanese
// corrective guidance." This function is the one place in the engine that
// is allowed to decide Pass vs. Retry vs. SpecificError, and it is
// deliberately conservative: no audio-quality pass, no diagnosis; no
// content-authored confidence threshold cleared, no diagnosis.
Diagnosis decideDiagnosis(const AudioQualityResult& audioQuality,
                           const std::vector<PhonemeScoreResult>& phonemeScores,
                           const ExerciseDefinition& exercise,
                           const ContentStore& content,
                           const std::vector<GlobalPatternCandidate>& globalCandidates,
                           ErrorHistory& history);

} // namespace pronunciation
