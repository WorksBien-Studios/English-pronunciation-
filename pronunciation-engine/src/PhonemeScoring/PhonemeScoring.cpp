#include "PhonemeScoring.h"

#include <cmath>
#include <limits>

namespace pronunciation {

namespace {

float averageLogProb(const FrameLogProbs& logProbs, int column, int startFrame, int endFrame) {
    double sum = 0.0;
    int count = 0;
    for (int t = startFrame; t <= endFrame; ++t) {
        sum += logProbs.at(t, column);
        ++count;
    }
    return count > 0 ? static_cast<float>(sum / count) : -std::numeric_limits<float>::infinity();
}

} // namespace

std::vector<PhonemeScoreResult> scorePhonemes(const FrameLogProbs& logProbs,
                                               const ForcedAlignmentResult& alignment,
                                               const ExerciseDefinition& exercise,
                                               const PhonemeScoringThresholds& thresholds) {
    std::vector<PhonemeScoreResult> results;
    results.reserve(alignment.phonemes.size());

    for (const auto& aligned : alignment.phonemes) {
        PhonemeScoreResult score;
        score.expectedIndex = aligned.expectedIndex;
        score.expectedPhonemeId = aligned.phonemeId;

        if (!aligned.hasFrames()) {
            score.verdict = PhonemeVerdict::Deleted;
            score.targetPosterior = 0.0f;
            results.push_back(score);
            continue;
        }

        float avgTargetLogProb = averageLogProb(logProbs, aligned.phonemeId, aligned.startFrame, aligned.endFrame);
        score.targetPosterior = std::exp(avgTargetLogProb);

        const auto& slot = exercise.expectedPhonemes[static_cast<size_t>(aligned.expectedIndex)];
        std::vector<PhonemeId> competitorPool = slot.confusablePhonemeIds;
        if (competitorPool.empty()) {
            // No content-authored confusion set for this slot: fall back to
            // scanning every other phoneme so an unexpected, severe
            // mismatch is still detectable rather than silently ignored.
            for (int v = 0; v < logProbs.vocabSize; ++v) {
                if (v != aligned.phonemeId && v != logProbs.blankColumn) competitorPool.push_back(v);
            }
        }

        PhonemeId bestCompetitor = kInvalidPhonemeId;
        float bestCompetitorLogProb = -std::numeric_limits<float>::infinity();
        for (PhonemeId competitor : competitorPool) {
            float avg = averageLogProb(logProbs, competitor, aligned.startFrame, aligned.endFrame);
            if (avg > bestCompetitorLogProb) {
                bestCompetitorLogProb = avg;
                bestCompetitor = competitor;
            }
        }
        score.competitorPhonemeId = bestCompetitor;
        score.competitorPosterior = bestCompetitor == kInvalidPhonemeId ? 0.0f : std::exp(bestCompetitorLogProb);
        score.gopScore = avgTargetLogProb - bestCompetitorLogProb;

        if (score.targetPosterior < thresholds.deletionPosteriorFloor) {
            score.verdict = PhonemeVerdict::Deleted;
        } else if (score.competitorPosterior > score.targetPosterior &&
                   score.competitorPosterior >= thresholds.substitutionPosteriorFloor) {
            score.verdict = PhonemeVerdict::Substituted;
        } else if (score.targetPosterior >= thresholds.correctPosteriorFloor && score.gopScore > 0.0f) {
            score.verdict = PhonemeVerdict::Correct;
        } else {
            score.verdict = PhonemeVerdict::LowConfidence;
        }

        results.push_back(score);
    }

    return results;
}

} // namespace pronunciation
