#pragma once

#include <memory>
#include <string>

#include "AcousticModel.h"

namespace pronunciation {

// Production acoustic backend for the frozen
// onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX model.
//
// The backend accepts the recorder's mono PCM at its native sample rate,
// resamples to the model's required 16 kHz, applies the same zero-mean /
// unit-variance normalization as Wav2Vec2FeatureExtractor, runs ONNX Runtime,
// then conservatively collapses the model's 392 multilingual IPA labels into
// the engine inventory using Resources/acoustic-token-map.json.
//
// Unsupported model labels are folded into CTC blank rather than into a
// guessed English phoneme. Core ML is requested on Apple platforms and ONNX
// Runtime's CPU provider remains the fallback.
class OnnxAcousticModel : public AcousticModel {
public:
    OnnxAcousticModel(std::string modelPath, std::string resourcesDir);
    ~OnnxAcousticModel() override;

    OnnxAcousticModel(const OnnxAcousticModel&) = delete;
    OnnxAcousticModel& operator=(const OnnxAcousticModel&) = delete;
    OnnxAcousticModel(OnnxAcousticModel&&) noexcept;
    OnnxAcousticModel& operator=(OnnxAcousticModel&&) noexcept;

    FrameLogProbs infer(const PcmBuffer& audio) override;
    int vocabularySize() const override;
    int blankColumn() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::string modelPath_;
    std::string resourcesDir_;
    int vocabularySize_ = 0;
    int blankColumn_ = 0;
};

} // namespace pronunciation
