#include "OnnxAcousticModel.h"

#include <stdexcept>

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
#error "ONNX Runtime backend is not implemented yet; see OnnxAcousticModel.h."
#endif

namespace pronunciation {

OnnxAcousticModel::OnnxAcousticModel(std::string modelPath) : modelPath_(std::move(modelPath)) {}

OnnxAcousticModel::~OnnxAcousticModel() = default;

FrameLogProbs OnnxAcousticModel::infer(const PcmBuffer&) {
    throw std::runtime_error(
        "OnnxAcousticModel is not wired in this build. Build with "
        "PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME and link ONNX Runtime Mobile "
        "against the bundled model at " + modelPath_ + " (see model/model-manifest.json). "
        "Use MockAcousticModel for engine development and tests until then.");
}

int OnnxAcousticModel::vocabularySize() const {
    throw std::runtime_error("OnnxAcousticModel is not wired in this build.");
}

int OnnxAcousticModel::blankColumn() const {
    throw std::runtime_error("OnnxAcousticModel is not wired in this build.");
}

} // namespace pronunciation
