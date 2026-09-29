#include "OnnxAcousticModel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
#if defined(__APPLE__)
// The official Apple archive is linked as onnxruntime.framework. Framework
// headers are addressed through the framework name rather than as flat
// headers, unlike the Linux package used by the integration smoke test.
#include <onnxruntime/onnxruntime_cxx_api.h>
#else
#include <onnxruntime_cxx_api.h>
#endif
#endif

#include "ModelOutputProjection.h"

namespace pronunciation {

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME

namespace {

constexpr int kRequiredSampleRateHz = 16000;

std::vector<float> resampleTo16k(const PcmBuffer& audio) {
    if (audio.sampleRateHz <= 0 || audio.samples.empty()) {
        throw std::invalid_argument("OnnxAcousticModel: audio must contain samples at a valid rate");
    }
    for (float sample : audio.samples) {
        if (!std::isfinite(sample)) {
            throw std::invalid_argument("OnnxAcousticModel: audio contains non-finite samples");
        }
    }
    if (audio.sampleRateHz == kRequiredSampleRateHz) return audio.samples;

    const double ratio = static_cast<double>(kRequiredSampleRateHz) /
                         static_cast<double>(audio.sampleRateHz);
    const size_t outputCount = std::max<size_t>(
        1, static_cast<size_t>(std::llround(audio.samples.size() * ratio)));
    std::vector<float> output(outputCount);
    if (audio.samples.size() == 1) {
        std::fill(output.begin(), output.end(), audio.samples.front());
        return output;
    }

    const double sourceStep = static_cast<double>(audio.sampleRateHz) /
                              static_cast<double>(kRequiredSampleRateHz);
    for (size_t index = 0; index < outputCount; ++index) {
        const double sourcePosition = static_cast<double>(index) * sourceStep;
        const size_t left = std::min(
            static_cast<size_t>(sourcePosition), audio.samples.size() - 1);
        const size_t right = std::min(left + 1, audio.samples.size() - 1);
        const float fraction = static_cast<float>(sourcePosition - left);
        output[index] = audio.samples[left] +
                        (audio.samples[right] - audio.samples[left]) * fraction;
    }
    return output;
}

void normalize(std::vector<float>& samples) {
    if (samples.empty()) return;
    double mean = 0.0;
    for (float sample : samples) mean += sample;
    mean /= static_cast<double>(samples.size());

    double variance = 0.0;
    for (float sample : samples) {
        const double centered = static_cast<double>(sample) - mean;
        variance += centered * centered;
    }
    variance /= static_cast<double>(samples.size());
    const double scale = 1.0 / std::sqrt(variance + 1e-7);
    for (float& sample : samples) {
        sample = static_cast<float>((static_cast<double>(sample) - mean) * scale);
    }
}

} // namespace

class OnnxAcousticModel::Impl {
public:
    Impl(const std::string& modelPath, const std::string& resourcesDirectory)
        : projection(ModelOutputProjection::loadFromFiles(
              resourcesDirectory + "/phonemes.json",
              resourcesDirectory + "/model-output-map.json")),
          environment(ORT_LOGGING_LEVEL_WARNING, "pronunciation-engine") {
        if (!std::filesystem::is_regular_file(modelPath)) {
            throw std::runtime_error("OnnxAcousticModel: model file is missing at " + modelPath);
        }

        Ort::SessionOptions options;
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        options.SetIntraOpNumThreads(2);

#if defined(__APPLE__)
        // Unsupported nodes (including quantized contrib operators) remain on
        // the CPU provider automatically. If Core ML is unavailable, session
        // creation still proceeds with the default CPU provider.
        try {
            options.AppendExecutionProvider(
                "CoreML",
                {{"ModelFormat", "MLProgram"},
                 {"MLComputeUnits", "ALL"},
                 {"RequireStaticInputShapes", "0"}});
        } catch (const Ort::Exception&) {
        }
#endif

        session = std::make_unique<Ort::Session>(
            environment, modelPath.c_str(), options);
        Ort::AllocatorWithDefaultOptions allocator;
        if (session->GetInputCount() != 1 || session->GetOutputCount() != 1) {
            throw std::runtime_error(
                "OnnxAcousticModel: expected exactly one model input and one output");
        }
        auto input = session->GetInputNameAllocated(0, allocator);
        auto output = session->GetOutputNameAllocated(0, allocator);
        inputName = input.get();
        outputName = output.get();
        if (inputName != "input_values" || outputName != "logits") {
            throw std::runtime_error(
                "OnnxAcousticModel: unexpected model input/output names");
        }
    }

    ModelOutputProjection projection;
    Ort::Env environment;
    std::unique_ptr<Ort::Session> session;
    std::string inputName;
    std::string outputName;
};

#else

class OnnxAcousticModel::Impl {};

#endif

OnnxAcousticModel::OnnxAcousticModel(
    std::string modelPath,
    std::string resourcesDirectory)
    : modelPath_(std::move(modelPath)),
      resourcesDirectory_(std::move(resourcesDirectory)) {
#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    impl_ = std::make_unique<Impl>(modelPath_, resourcesDirectory_);
#endif
}

OnnxAcousticModel::~OnnxAcousticModel() = default;

FrameLogProbs OnnxAcousticModel::infer(const PcmBuffer& audio) {
#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    if (!impl_ || !impl_->session) {
        throw std::runtime_error("OnnxAcousticModel: runtime session is unavailable");
    }

    std::vector<float> inputSamples = resampleTo16k(audio);
    normalize(inputSamples);
    const std::array<int64_t, 2> inputShape = {
        1, static_cast<int64_t>(inputSamples.size())};
    Ort::MemoryInfo memory = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memory,
        inputSamples.data(),
        inputSamples.size(),
        inputShape.data(),
        inputShape.size());
    const char* inputNames[] = {impl_->inputName.c_str()};
    const char* outputNames[] = {impl_->outputName.c_str()};
    auto outputs = impl_->session->Run(
        Ort::RunOptions{nullptr},
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1);
    if (outputs.size() != 1 || !outputs.front().IsTensor()) {
        throw std::runtime_error("OnnxAcousticModel: model did not return a logits tensor");
    }

    const auto info = outputs.front().GetTensorTypeAndShapeInfo();
    if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        throw std::runtime_error("OnnxAcousticModel: logits tensor must contain float32 values");
    }
    const std::vector<int64_t> shape = info.GetShape();
    if (shape.size() != 3 || shape[0] != 1 || shape[1] <= 0 ||
        shape[1] > std::numeric_limits<int>::max() ||
        shape[2] != impl_->projection.modelVocabularySize()) {
        throw std::runtime_error("OnnxAcousticModel: unexpected logits tensor shape");
    }
    const int numFrames = static_cast<int>(shape[1]);
    const double frameDuration =
        static_cast<double>(inputSamples.size()) /
        static_cast<double>(kRequiredSampleRateHz) /
        static_cast<double>(numFrames);
    return impl_->projection.project(
        outputs.front().GetTensorData<float>(),
        numFrames,
        static_cast<int>(shape[2]),
        frameDuration);
#else
    (void)audio;
    throw std::runtime_error(
        "OnnxAcousticModel is not wired in this build. Build with "
        "PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME and link ONNX Runtime against "
        "the bundled model at " + modelPath_ + ".");
#endif
}

int OnnxAcousticModel::vocabularySize() const {
#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    return impl_->projection.vocabularySize();
#else
    throw std::runtime_error("OnnxAcousticModel is not wired in this build.");
#endif
}

int OnnxAcousticModel::blankColumn() const {
#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    return impl_->projection.blankColumn();
#else
    throw std::runtime_error("OnnxAcousticModel is not wired in this build.");
#endif
}

} // namespace pronunciation
