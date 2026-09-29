#include "Engine.h"

#include <algorithm>
#include <stdexcept>

#include "CTCAlignment/CTCAlignment.h"
#include "PhonemeScoring/PhonemeScoring.h"
#include "Prosody/Prosody.h"

namespace pronunciation {

namespace {

// Splits "prefix:rest" into {prefix, rest}.
std::pair<std::string, std::string> splitOnce(const std::string& s, char sep) {
    auto pos = s.find(sep);
    if (pos == std::string::npos) throw std::runtime_error("Engine: malformed exercise id " + s);
    return {s.substr(0, pos), s.substr(pos + 1)};
}

} // namespace

PronunciationEngine::PronunciationEngine(const std::string& resourcesDir, std::unique_ptr<AcousticModel> model)
    : inventory_(PhonemeInventory::loadFromFile(resourcesDir + "/phonemes.json")),
      content_(ContentStore::loadFromDirectory(resourcesDir, inventory_)),
      model_(std::move(model)) {}

ExerciseDefinition PronunciationEngine::resolveExercise(const std::string& exerciseId) const {
    auto [kind, rest] = splitOnce(exerciseId, ':');
    if (kind == "word") return content_.buildExerciseForWord(rest);
    if (kind == "sentence") return content_.buildExerciseForSentence(rest);
    if (kind == "minimalPair") {
        auto [pairId, side] = splitOnce(rest, ':');
        if (side != "A" && side != "B") throw std::runtime_error("Engine: minimal pair side must be A or B, got " + side);
        return content_.buildExerciseForMinimalPair(pairId, side == "A");
    }
    throw std::runtime_error("Engine: unknown exercise kind " + kind);
}

std::vector<GlobalPatternCandidate> PronunciationEngine::detectGlobalPatterns(
    const FrameLogProbs& logProbs, const ExerciseDefinition& exercise, const std::vector<PhonemeScoreResult>& phonemeScores) const {
    std::vector<GlobalPatternCandidate> candidates;
    if (exercise.expectedPhonemes.empty() || phonemeScores.empty()) return candidates;

    // Final-consonant deletion: the aligner was forced to allocate frames to
    // the last expected phoneme, but PhonemeScoring found essentially no
    // acoustic support for it, and no substitution pattern already claims
    // this slot.
    const auto& lastSlot = exercise.expectedPhonemes.back();
    const auto& lastScore = phonemeScores.back();
    if (!lastSlot.isVowel && lastSlot.errorPatternId.empty() && lastScore.verdict == PhonemeVerdict::Deleted) {
        candidates.push_back({"final-consonant-deletion", std::clamp(1.0f - lastScore.targetPosterior, 0.0f, 1.0f)});
    }

    // Epenthetic vowel insertion: an unconstrained greedy decode finds an
    // extra vowel appended after the entire expected sequence has already
    // been matched off (a forced alignment constrained to the expected
    // sequence cannot represent this by construction, hence the separate
    // free decode here).
    std::vector<PhonemeId> reference;
    reference.reserve(exercise.expectedPhonemes.size());
    for (const auto& slot : exercise.expectedPhonemes) reference.push_back(slot.phonemeId);

    std::vector<PhonemeId> hypothesis = greedyDecode(logProbs);
    std::vector<EditOp> ops = levenshteinAlign(hypothesis, reference);
    if (!ops.empty() && ops.back().type == EditOpType::Insert) {
        PhonemeId insertedId = hypothesis[static_cast<size_t>(ops.back().hypothesisIndex)];
        auto info = inventory_.infoForId(insertedId);
        if (info && info->isVowel) {
            candidates.push_back({"vowel-insertion-after-consonant", 0.75f});
        }
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
        // Too little acoustic evidence to even attempt scoring (e.g. the
        // recording is shorter than the expected phoneme sequence
        // requires). Never guess; ask for a retry.
        result.diagnosis.outcome = DiagnosisOutcome::Retry;
        result.diagnosis.confidence = ConfidenceLevel::Low;
        return result;
    }

    result.phonemeScores = scorePhonemes(logProbs, result.alignment, exercise);
    result.prosody = extractProsody(audio, result.alignment, exercise, logProbs.frameDurationSeconds);

    std::vector<GlobalPatternCandidate> globalCandidates = detectGlobalPatterns(logProbs, exercise, result.phonemeScores);
    result.diagnosis = decideDiagnosis(result.audioQuality, result.phonemeScores, exercise, content_, globalCandidates, history_);

    return result;
}

} // namespace pronunciation
