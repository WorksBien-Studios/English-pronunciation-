#pragma once

#include <memory>
#include <string>

#include "AcousticModel.h"

namespace pronunciation {

// Production backend wiring point for the locked model
// (onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX, model_q4f16.onnx —
// see model/model-manifest.json) run through ONNX Runtime Mobile with the
// Core ML execution provider (docs/product-specification.md#runtime-inference).
//
// The model emits a 392-label eSpeak/IPA vocabulary. Its logits are projected
// through Resources/model-output-map.json into the engine's inventory plus
// explicit unknown and CTC-blank columns before any alignment or scoring.
//
// Build with -DPRONUNCIATION_ENGINE_WITH_ONNXRUNTIME=ON and provide ONNX
// Runtime's headers/library to enable inference. Builds without ONNX Runtime
// retain a fail-closed stub so the portable unit suite needs no model binary.
class OnnxAcousticModel : public AcousticModel {
public:
    OnnxAcousticModel(std::string modelPath, std::string resourcesDirectory);
    ~OnnxAcousticModel() override;

    FrameLogProbs infer(const PcmBuffer& audio) override;
    int vocabularySize() const override;
    int blankColumn() const override;

private:
    class Impl;
    std::string modelPath_;
    std::string resourcesDirectory_;
    std::unique_ptr<Impl> impl_;
};

} // namespace pronunciation
