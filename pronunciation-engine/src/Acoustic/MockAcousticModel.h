#pragma once

#include <vector>

#include "AcousticModel.h"

namespace pronunciation {

// One scripted acoustic frame: "the model is `dominantProbability` confident
// the sound in this frame is `dominantPhonemeId`", with the remaining
// probability mass spread uniformly over blank and every other class. A
// dominantPhonemeId of kBlankPhonemeId scripts a blank/silence frame.
struct ScriptedFrame {
    PhonemeId dominantPhonemeId = kBlankPhonemeId;
    float dominantProbability = 0.9f;
};

// A deterministic AcousticModel stand-in used by unit tests and the CLI
// harness. It ignores the PCM audio entirely and instead replays a
// pre-scripted frame sequence, which is what lets the alignment/scoring/
// decision-rule tests exercise "correct pronunciation", "substituted
// phoneme", "deleted phoneme", and "inserted phoneme" scenarios without a
// real recording or the bundled ONNX model.
class MockAcousticModel : public AcousticModel {
public:
    MockAcousticModel(int vocabularySize, int blankColumn, double frameDurationSeconds = 0.02);

    void setScript(std::vector<ScriptedFrame> frames);

    // Convenience: repeats each entry in `sequence` for `framesPerPhoneme`
    // frames, in order. This is the common case for "produced roughly this
    // phoneme sequence at roughly this confidence".
    void setScriptFromSequence(const std::vector<ScriptedFrame>& sequence, int framesPerPhoneme);

    FrameLogProbs infer(const PcmBuffer& audio) override;
    int vocabularySize() const override { return vocabularySize_; }
    int blankColumn() const override { return blankColumn_; }

private:
    int vocabularySize_;
    int blankColumn_;
    double frameDurationSeconds_;
    std::vector<ScriptedFrame> script_;
};

} // namespace pronunciation
