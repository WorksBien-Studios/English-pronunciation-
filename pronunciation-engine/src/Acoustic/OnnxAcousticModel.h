#pragma once

#include <string>

#include "AcousticModel.h"

namespace pronunciation {

// Production backend wiring point for the locked model
// (onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX, model_q4f16.onnx —
// see model/model-manifest.json) run through ONNX Runtime Mobile with the
// Core ML execution provider (docs/product-specification.md#runtime-inference).
//
// This class is intentionally NOT implemented yet: it requires linking
// ONNX Runtime and bundling the ~197 MB model artifact, both of which
// belong to the iOS target build (pronunciation-ios/EngineBridge), not to
// engine development in this environment. Every other module in this
// engine (AudioQuality, CTCAlignment, PhonemeScoring, Prosody,
// DecisionRules, ContentValidation) only depends on the AcousticModel
// interface, so wiring this in later does not require touching them.
//
// Build with -DPRONUNCIATION_ENGINE_WITH_ONNXRUNTIME=ON and provide
// onnxruntime's headers/libs to enable the real implementation; otherwise
// infer() throws so the mistake is loud rather than silently mis-scoring.
class OnnxAcousticModel : public AcousticModel {
public:
    explicit OnnxAcousticModel(std::string modelPath);
    ~OnnxAcousticModel() override;

    FrameLogProbs infer(const PcmBuffer& audio) override;
    int vocabularySize() const override;
    int blankColumn() const override;

private:
    std::string modelPath_;
};

} // namespace pronunciation
