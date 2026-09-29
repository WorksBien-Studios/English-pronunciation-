#include "MockAcousticModel.h"

#include <cmath>

namespace pronunciation {

MockAcousticModel::MockAcousticModel(int vocabularySize, int blankColumn, double frameDurationSeconds)
    : vocabularySize_(vocabularySize), blankColumn_(blankColumn), frameDurationSeconds_(frameDurationSeconds) {}

void MockAcousticModel::setScript(std::vector<ScriptedFrame> frames) {
    script_ = std::move(frames);
}

void MockAcousticModel::setScriptFromSequence(const std::vector<ScriptedFrame>& sequence, int framesPerPhoneme) {
    script_.clear();
    script_.reserve(sequence.size() * static_cast<size_t>(framesPerPhoneme));
    for (const auto& entry : sequence) {
        for (int i = 0; i < framesPerPhoneme; ++i) script_.push_back(entry);
    }
}

FrameLogProbs MockAcousticModel::infer(const PcmBuffer&) {
    FrameLogProbs out;
    out.vocabSize = vocabularySize_;
    out.blankColumn = blankColumn_;
    out.frameDurationSeconds = frameDurationSeconds_;
    out.numFrames = static_cast<int>(script_.size());
    out.data.assign(static_cast<size_t>(out.numFrames) * out.vocabSize, 0.0f);

    for (int t = 0; t < out.numFrames; ++t) {
        const ScriptedFrame& frame = script_[static_cast<size_t>(t)];
        int dominantColumn = frame.dominantPhonemeId == kBlankPhonemeId ? blankColumn_ : frame.dominantPhonemeId;
        float dominant = std::min(std::max(frame.dominantProbability, 1e-4f), 1.0f - 1e-4f);
        float remainder = (1.0f - dominant) / static_cast<float>(vocabularySize_ - 1);
        for (int v = 0; v < vocabularySize_; ++v) {
            float p = (v == dominantColumn) ? dominant : remainder;
            out.data[static_cast<size_t>(t) * vocabularySize_ + v] = std::log(p);
        }
    }
    return out;
}

} // namespace pronunciation
