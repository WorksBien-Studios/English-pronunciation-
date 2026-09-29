#pragma once

#include <memory>
#include <string>

#include "AudioQuality/AudioQuality.h"
#include "Acoustic/AcousticModel.h"
#include "Content/ContentStore.h"
#include "Content/PhonemeInventory.h"
#include "DecisionRules/DecisionRules.h"
#include "DecisionRules/ErrorHistory.h"
#include "Types.h"

namespace pronunciation {

// Orchestrates the locked ten-step engine pipeline
// (docs/product-specification.md#model-strategy) over one exercise
// attempt. This is the class the narrow C API (include/PronunciationEngine.h)
// wraps for Swift; it has no SwiftUI/SwiftData/StoreKit dependency, per the
// "engine-first" architecture rule.
class PronunciationEngine {
public:
    // resourcesDir must contain phonemes.json plus the rest of the content
    // pack (see Resources/). `model` supplies frame-level phoneme
    // log-probabilities; pass a MockAcousticModel for development/tests and
    // an OnnxAcousticModel once the ONNX Runtime backend is wired for iOS.
    PronunciationEngine(const std::string& resourcesDir, std::unique_ptr<AcousticModel> model);

    // exerciseId must be one of the forms produced by ContentStore's
    // buildExerciseFor*() helpers: "word:<id>", "minimalPair:<id>:A",
    // "minimalPair:<id>:B", or "sentence:<id>".
    EngineResult process(const PcmBuffer& audio, const std::string& exerciseId);

    const ContentStore& content() const { return content_; }
    const PhonemeInventory& inventory() const { return inventory_; }
    ErrorHistory& history() { return history_; }

private:
    PhonemeInventory inventory_;
    ContentStore content_;
    std::unique_ptr<AcousticModel> model_;
    AudioQualityGate qualityGate_;
    ErrorHistory history_;

    ExerciseDefinition resolveExercise(const std::string& exerciseId) const;
    std::vector<GlobalPatternCandidate> detectGlobalPatterns(const FrameLogProbs& logProbs,
                                                              const ExerciseDefinition& exercise,
                                                              const std::vector<PhonemeScoreResult>& phonemeScores) const;
};

} // namespace pronunciation
