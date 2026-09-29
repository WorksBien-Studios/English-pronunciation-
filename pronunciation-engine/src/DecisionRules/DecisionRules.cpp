#include "DecisionRules.h"

#include <algorithm>

namespace pronunciation {

namespace {

struct Candidate {
    std::string errorPatternId;
    float evidenceScore = 0.0f;
    float confidenceThreshold = 1.0f;
    int repeatsRequiredToConfirm = 2;
    PhonemeId expectedPhonemeId = kInvalidPhonemeId;
    PhonemeId producedPhonemeId = kInvalidPhonemeId;
    int expectedIndex = -1;
    bool clearsThreshold() const { return evidenceScore >= confidenceThreshold; }
};

std::string historyKey(const Candidate& candidate) {
    // Deletions have no meaningful "produced" phoneme, so the key is just
    // the pattern id; substitutions are keyed on the specific
    // expected->produced pair so distinct confusions under the same
    // pattern are tracked separately.
    if (candidate.producedPhonemeId == kInvalidPhonemeId) return candidate.errorPatternId;
    return candidate.errorPatternId + ":" + std::to_string(candidate.expectedPhonemeId) + "->" +
           std::to_string(candidate.producedPhonemeId);
}

Diagnosis retryLowConfidence() {
    Diagnosis diagnosis;
    diagnosis.outcome = DiagnosisOutcome::Retry;
    diagnosis.confidence = ConfidenceLevel::Low;
    return diagnosis;
}

} // namespace

Diagnosis decideDiagnosis(const AudioQualityResult& audioQuality,
                           const std::vector<PhonemeScoreResult>& phonemeScores,
                           const ExerciseDefinition& exercise,
                           const ContentStore& content,
                           const std::vector<GlobalPatternCandidate>& globalCandidates,
                           ErrorHistory& history) {
    if (!audioQuality.passesGate) return retryLowConfidence();

    std::vector<Candidate> candidates;
    bool sawLowConfidenceEvidence = false;
    bool sawUnmappedErrorEvidence = false;

    for (const auto& score : phonemeScores) {
        if (score.verdict == PhonemeVerdict::Correct) continue;

        if (score.verdict == PhonemeVerdict::LowConfidence) {
            sawLowConfidenceEvidence = true;
            continue;
        }

        // Insertions are handled as exercise-level global candidates. If one
        // ever reaches this per-slot surface without a mapped pattern, do not
        // silently convert it into a pass.
        if (score.verdict == PhonemeVerdict::Inserted) {
            sawUnmappedErrorEvidence = true;
            continue;
        }

        if (score.verdict != PhonemeVerdict::Substituted && score.verdict != PhonemeVerdict::Deleted) {
            sawLowConfidenceEvidence = true;
            continue;
        }

        if (score.expectedIndex < 0 || static_cast<size_t>(score.expectedIndex) >= exercise.expectedPhonemes.size()) {
            sawUnmappedErrorEvidence = true;
            continue;
        }

        const auto& slot = exercise.expectedPhonemes[static_cast<size_t>(score.expectedIndex)];
        if (slot.errorPatternId.empty()) {
            // A real mismatch can occur outside the authored Japanese-specific
            // pattern set. It is not safe to call that a correct pronunciation.
            sawUnmappedErrorEvidence = true;
            continue;
        }

        const JapaneseErrorPattern* pattern = content.findErrorPattern(slot.errorPatternId);
        if (!pattern) {
            sawUnmappedErrorEvidence = true;
            continue;
        }

        Candidate candidate;
        candidate.errorPatternId = pattern->id;
        candidate.confidenceThreshold = pattern->confidenceThreshold;
        candidate.repeatsRequiredToConfirm = pattern->repeatsRequiredToConfirm;
        candidate.expectedPhonemeId = score.expectedPhonemeId;
        candidate.expectedIndex = score.expectedIndex;
        candidate.evidenceScore = score.verdict == PhonemeVerdict::Substituted
                                      ? score.competitorPosterior
                                      : std::clamp(1.0f - score.targetPosterior, 0.0f, 1.0f);
        candidate.producedPhonemeId =
            score.verdict == PhonemeVerdict::Substituted ? score.competitorPhonemeId : kInvalidPhonemeId;
        candidates.push_back(candidate);
    }

    for (const auto& globalCandidate : globalCandidates) {
        if (const JapaneseErrorPattern* pattern = content.findErrorPattern(globalCandidate.errorPatternId)) {
            Candidate candidate;
            candidate.errorPatternId = pattern->id;
            candidate.confidenceThreshold = pattern->confidenceThreshold;
            candidate.repeatsRequiredToConfirm = pattern->repeatsRequiredToConfirm;
            candidate.evidenceScore = globalCandidate.evidenceScore;
            candidates.push_back(candidate);
        } else {
            sawUnmappedErrorEvidence = true;
        }
    }

    if (candidates.empty()) {
        // A non-empty exercise with no phoneme scores is insufficient
        // evidence, never a high-confidence pass.
        if ((!exercise.expectedPhonemes.empty() && phonemeScores.empty()) ||
            sawLowConfidenceEvidence || sawUnmappedErrorEvidence) {
            return retryLowConfidence();
        }

        Diagnosis diagnosis;
        diagnosis.outcome = DiagnosisOutcome::Pass;
        diagnosis.confidence = ConfidenceLevel::High;
        return diagnosis;
    }

    auto best = std::max_element(candidates.begin(), candidates.end(),
                                 [](const Candidate& a, const Candidate& b) {
                                     return a.evidenceScore < b.evidenceScore;
                                 });

    if (!best->clearsThreshold()) {
        // Evidence of a problem exists but never crosses the locked
        // threshold: never guess a specific physical correction.
        return retryLowConfidence();
    }

    Diagnosis diagnosis;
    diagnosis.outcome = DiagnosisOutcome::SpecificError;
    diagnosis.confidence = ConfidenceLevel::High;
    diagnosis.errorPatternId = best->errorPatternId;
    diagnosis.expectedPhonemeId = best->expectedPhonemeId;
    diagnosis.producedPhonemeId = best->producedPhonemeId;
    diagnosis.expectedIndex = best->expectedIndex;

    int newCount = history.recordHighConfidenceObservation(historyKey(*best));
    diagnosis.errorConfirmedByHistory = newCount >= best->repeatsRequiredToConfirm;
    return diagnosis;
}

} // namespace pronunciation
