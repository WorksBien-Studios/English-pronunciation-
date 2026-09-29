#pragma once

#include <memory>
#include <string>

#include "AcousticModel.h"

namespace pronunciation {

// Production acoustic backend for the locked Q4-FP16 Wav2Vec2 phoneme model.
// The upstream model exposes 392 eSpeak/IPA CTC labels. This adapter projects
// them into the engine's fixed 40-phoneme inventory plus one blank/unsupported
// bucket before any alignment or GOP scoring happens.
//
// Unsupported upstream labels are deliberately folded into blank rather than
// guessed into a nearby English phoneme. That makes inference conservative:
// evidence the engine does not understand causes retry/deletion pressure, not
// a fabricated high-confidence pronunciation result.
class OnnxAcousticModel : public AcousticModel {
public:
    explicit OnnxAcousticModel(std::string modelPath);
    ~OnnxAcousticModel() override;

    FrameLogProbs infer(const PcmBuffer& audio) override;
    int vocabularySize() const override;
    int blankColumn() const override;

private:
    struct Impl;

    std::string modelPath_;
    std::unique_ptr<Impl> impl_;
};

} // namespace pronunciation
