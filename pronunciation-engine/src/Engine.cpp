#include "Engine.h"

#include <algorithm>
#include <stdexcept>

#include "CTCAlignment/CTCAlignment.h"
#include "PhonemeScoring/PhonemeScoring.h"
#include "Prosody/Prosody.h"

namespace pronunciation {

namespace {

std::pair<std::string, std::string> splitOnce(const std::string& s, char sep) {
    auto pos = s.find(sep);
    if (pos == std::string::npos) throw std::runtime_error("Engine: malformed exercise id " + s);
    return {s.substr(0, pos), s.substr(pos + 1)};
}

} // namespace

PronunciationEngine::PronunciationEngine(const std::string& resourcesDir, std::unique_ptr<AcousticModel> model)
    : inventory_(PhonemeInventory::loadFromFile(resourcesDir + "/phonemes.json")),
      content_(ContentStore::loadFromDirectory(resourcesDir, inventory_)),
      model_(std::move(model)) {
    if (!model_) throw std::invalid_argument("Engine: acoustic model must not be null");
}

ExerciseDefinition PronunciationEngine::resolveExercise(const std::string& exerciseId) const {
    auto [kind, rest] = splitOnce(exerciseId, ':');
    if (kind == "word") return content_.buildExerciseForWord(rest);
    if (kind == "sentence") return content_.buildExerciseForSentence(rest);
    if (kind == "minimalPair") {
        auto [pairId, side] = splitOnce(rest, ':');
        if (side != "A" && side != "B") {
            throw std::runtime_error("Engine: minimal pair side must be A or B, got " + side);
        }
        return content_.buildExerciseForMinimalPair(pairId, side == "A");
    }
    throw std::runtime_error("Engine: unknown exercise kind " + kind);
}

std::vector<GlobalPatternCandidate> PronunciationEngine::detectGlobalPatterns(
    const FrameLogProbs& logProbs,
    const ExerciseDefinition& exercise,
    const std::vector<PhonemeScoreResult>& phonemeScores) const {
    std::vector<GlobalPatternCandidate> candidates;
    if (exercise.expectedPhonemes.empty() || phonemeScores.empty()) return candidates;

    const auto& lastSlot = exercise.expectedPhonemes.back();
    const auto& lastScore = phonemeScores.back();
    if (!lastSlot.isVowel && lastSlot.errorPatternId.empty() &&
        lastScore.verdict == PhonemeVerdict::Deleted) {
        candidates.push_back({
            "final-consonant-deletion",
            std::clamp(1.0f - lastScore.targetPosterior, 0.0f, 1.0f)
        });
    }

    std::vector<PhonemeId> reference;
    reference.reserve(exercise.expectedPhonemes.size());
    for (const auto& slot : exercise.expectedPhonemes) reference.push_back(slot.phonemeId);

    std::vector<PhonemeId> hypothesis = greedyDecode(logProbs);
    std::vector<EditOp> ops = levenshteinAlign(hypothesis, reference);

    // Detect epenthetic vowels anywhere after a consonant, not just at the
    // end of the utterance. Japanese learners may insert a vowel inside
    // consonant clusters as well as after a final consonant.
    int lastReferenceIndex = -1;
    for (const auto& op : ops) {
        if (op.type == EditOpType::Insert) {
            if (op.hypothesisIndex < 0 ||
                static_cast<size_t>(op.hypothesisIndex) >= hypothesis.size() ||
                lastReferenceIndex < 0 ||
                static_cast<size_t>(lastReferenceIndex) >= exercise.expectedPhonemes.size()) {
                continue;
            }

            const auto insertedInfo = inventory_.infoForId(
                hypothesis[static_cast<size_t>(op.hypothesisIndex)]);
            const auto& previousSlot =
                exercise.expectedPhonemes[static_cast<size_t>(lastReferenceIndex)];

            if (insertedInfo && insertedInfo->isVowel && !previousSlot.isVowel) {
                candidates.push_back({"vowel-insertion-after-consonant", 0.75f});
                break;
            }
            continue;
        }

        if (op.referenceIndex >= 0) lastReferenceIndex = op.referenceIndex;
    }

    return candidates;
}

EngineResult PronunciationEngine::process(const PcmBuffer& audio, const std::string& exerciseId) {
    EngineResult result;
    result.audioQuality = qualityGate_.evaluate(audio);
    if (!result.audioQuality.passesGate) {
        result.diagnosis = decideDiagnosis(result.audioQuality, {}, {}, content_, {}, history_);
        return result;
    }

    ExerciseDefinition exercise = resolveExercise(exerciseId);
    std::vector<PhonemeId> expectedIds;
    expectedIds.reserve(exercise.expectedPhonemes.size());
    for (const auto& slot : exercise.expectedPhonemes) expectedIds.push_back(slot.phonemeId);

    FrameLogProbs logProbs = model_->infer(audio);
    result.alignment = forceAlign(logProbs, expectedIds);
    if (!result.alignment.succeeded) {
        result.diagnosis.outcome = DiagnosisOutcome::Retry;
        result.diagnosis.confidence = ConfidenceLevel::Low;
        return result;
    }

    result.phonemeScores = scorePhonemes(logProbs, result.alignment, exercise);
    result.prosody = extractProsody(
        audio, result.alignment, exercise, logProbs.frameDurationSeconds);

    std::vector<GlobalPatternCandidate> globalCandidates =
        detectGlobalPatterns(logProbs, exercise, result.phonemeScores);
    result.diagnosis = decideDiagnosis(
        result.audioQuality, result.phonemeScores, exercise, content_,
        globalCandidates, history_);

    return result;
}

} // namespace pronunciation
