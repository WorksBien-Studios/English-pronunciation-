#pragma once

#include "../Types.h"

namespace pronunciation {

// Abstraction over "PCM audio in, frame-level phoneme log-probabilities
// out" so the CTC alignment / GOP scoring / decision layers never depend on
// a specific inference backend. The locked production backend is ONNX
// Runtime Mobile running the quantized Wav2Vec2 phoneme CTC checkpoint
// (see OnnxAcousticModel.h); MockAcousticModel drives the same interface
// deterministically for engine development, unit tests, and the CLI
// harness before the real model is wired into an iOS target.
//
// FrameLogProbs::data holds natural-log probabilities, already normalized
// per frame across the vocabulary (i.e. log(softmax(raw_logits))), not raw
// pre-softmax logits.
class AcousticModel {
public:
    virtual ~AcousticModel() = default;
    virtual FrameLogProbs infer(const PcmBuffer& audio) = 0;
    virtual int vocabularySize() const = 0;
    virtual int blankColumn() const = 0;
};

} // namespace pronunciation
