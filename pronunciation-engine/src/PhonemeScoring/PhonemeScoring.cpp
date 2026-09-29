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
    return count > 0 ? static_cast<float>(sum / count)
                     : -std::numeric_limits<float>::infinity();
}

bool validTensor(const FrameLogProbs& logProbs) {
    if (logProbs.numFrames < 0 || logProbs.vocabSize <= 0) return false;
    if (logProbs.blankColumn < 0 || logProbs.blankColumn >= logProbs.vocabSize) return false;

    const size_t frames = static_cast<size_t>(logProbs.numFrames);
    const size_t vocab = static_cast<size_t>(logProbs.vocabSize);
    if (vocab != 0 && frames > std::numeric_limits<size_t>::max() / vocab) return false;
    if (logProbs.data.size() != frames * vocab) return false;

    for (float value : logProbs.data) {
        if (std::isnan(value) || value == std::numeric_limits<float>::infinity()) return false;
    }
    return true;
}

PhonemeScoreResult lowConfidenceFor(const AlignedPhoneme& aligned) {
    PhonemeScoreResult score;
    score.expectedIndex = aligned.expectedIndex;
    score.expectedPhonemeId = aligned.phonemeId;
    score.verdict = PhonemeVerdict::LowConfidence;
    return score;
}

} // namespace

std::vector<PhonemeScoreResult> scorePhonemes(
    const FrameLogProbs& logProbs,
    const ForcedAlignmentResult& alignment,
    const ExerciseDefinition& exercise,
    const PhonemeScoringThresholds& thresholds) {

    std::vector<PhonemeScoreResult> results;
    results.reserve(alignment.phonemes.size());

    const bool tensorValid = validTensor(logProbs);

    for (const auto& aligned : alignment.phonemes) {
        if (!tensorValid ||
            aligned.expectedIndex < 0 ||
            static_cast<size_t>(aligned.expectedIndex) >= exercise.expectedPhonemes.size() ||
            aligned.phonemeId < 0 ||
            aligned.phonemeId >= logProbs.vocabSize ||
            aligned.phonemeId == logProbs.blankColumn ||
            (aligned.hasFrames() &&
             (aligned.startFrame < 0 ||
              aligned.endFrame >= logProbs.numFrames ||
              aligned.endFrame < aligned.startFrame))) {
            results.push_back(lowConfidenceFor(aligned));
            continue;
        }

        PhonemeScoreResult score;
        score.expectedIndex = aligned.expectedIndex;
        score.expectedPhonemeId = aligned.phonemeId;

        if (!aligned.hasFrames()) {
            score.verdict = PhonemeVerdict::Deleted;
            score.targetPosterior = 0.0f;
            results.push_back(score);
            continue;
        }

        float avgTargetLogProb =
            averageLogProb(logProbs, aligned.phonemeId, aligned.startFrame, aligned.endFrame);
        score.targetPosterior = std::exp(avgTargetLogProb);

        const auto& slot =
            exercise.expectedPhonemes[static_cast<size_t>(aligned.expectedIndex)];
        std::vector<PhonemeId> competitorPool = slot.confusablePhonemeIds;
        if (competitorPool.empty()) {
            for (int v = 0; v < logProbs.vocabSize; ++v) {
                if (v != aligned.phonemeId && v != logProbs.blankColumn) {
                    competitorPool.push_back(v);
                }
            }
        }

        PhonemeId bestCompetitor = kInvalidPhonemeId;
        float bestCompetitorLogProb = -std::numeric_limits<float>::infinity();
        for (PhonemeId competitor : competitorPool) {
            if (competitor < 0 || competitor >= logProbs.vocabSize ||
                competitor == logProbs.blankColumn) {
                continue;
            }
            float avg = averageLogProb(
                logProbs, competitor, aligned.startFrame, aligned.endFrame);
            if (avg > bestCompetitorLogProb) {
                bestCompetitorLogProb = avg;
                bestCompetitor = competitor;
            }
        }

        score.competitorPhonemeId = bestCompetitor;
        score.competitorPosterior =
            bestCompetitor == kInvalidPhonemeId ? 0.0f : std::exp(bestCompetitorLogProb);
        score.gopScore =
            bestCompetitor == kInvalidPhonemeId
                ? std::numeric_limits<float>::infinity()
                : avgTargetLogProb - bestCompetitorLogProb;

        if (!std::isfinite(score.targetPosterior) ||
            (!std::isfinite(score.competitorPosterior) && bestCompetitor != kInvalidPhonemeId)) {
            score.verdict = PhonemeVerdict::LowConfidence;
        } else if (score.targetPosterior < thresholds.deletionPosteriorFloor) {
            score.verdict = PhonemeVerdict::Deleted;
        } else if (score.competitorPosterior > score.targetPosterior &&
                   score.competitorPosterior >= thresholds.substitutionPosteriorFloor) {
            score.verdict = PhonemeVerdict::Substituted;
        } else if (score.targetPosterior >= thresholds.correctPosteriorFloor &&
                   score.gopScore > 0.0f) {
            score.verdict = PhonemeVerdict::Correct;
        } else {
            score.verdict = PhonemeVerdict::LowConfidence;
        }

        results.push_back(score);
    }

    return results;
}

} // namespace pronunciation
