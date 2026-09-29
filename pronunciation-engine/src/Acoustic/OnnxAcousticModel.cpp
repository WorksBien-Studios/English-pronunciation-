#include "OnnxAcousticModel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "../Content/PhonemeInventory.h"

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
#include <onnxruntime_cxx_api.h>
#endif

namespace pronunciation {
namespace {

constexpr int kModelSampleRate = 16000;
constexpr float kLogZero = -std::numeric_limits<float>::infinity();

struct TokenMapping {
    int modelVocabSize = 0;
    int blankTokenId = -1;
    int internalVocabSize = 0;
    int internalBlankColumn = 0;
    std::vector<std::vector<int>> tokenToInternal;
};

float logAdd(float a, float b) {
    if (!std::isfinite(a)) return b;
    if (!std::isfinite(b)) return a;
    const float hi = std::max(a, b);
    const float lo = std::min(a, b);
    return hi + std::log1p(std::exp(lo - hi));
}

TokenMapping loadTokenMapping(const std::string& resourcesDir) {
    PhonemeInventory inventory =
        PhonemeInventory::loadFromFile(resourcesDir + "/phonemes.json");

    std::ifstream in(resourcesDir + "/acoustic-token-map.json");
    if (!in) {
        throw std::runtime_error(
            "OnnxAcousticModel: could not open acoustic-token-map.json");
    }

    nlohmann::json doc;
    in >> doc;

    TokenMapping out;
    out.modelVocabSize = doc.at("modelVocabSize").get<int>();
    out.blankTokenId = doc.at("blankTokenId").get<int>();
    out.internalBlankColumn = static_cast<int>(inventory.size());
    out.internalVocabSize = out.internalBlankColumn + 1;

    if (out.modelVocabSize <= 0 ||
        out.blankTokenId < 0 ||
        out.blankTokenId >= out.modelVocabSize) {
        throw std::runtime_error(
            "OnnxAcousticModel: invalid acoustic token-map metadata");
    }

    out.tokenToInternal.resize(static_cast<size_t>(out.modelVocabSize));
    std::vector<bool> assigned(static_cast<size_t>(out.modelVocabSize), false);

    for (const auto& entry : doc.at("mappings")) {
        const auto& phonemes = entry.at("phonemes");
        if (phonemes.empty()) {
            throw std::runtime_error(
                "OnnxAcousticModel: empty phoneme list in acoustic token map");
        }

        std::vector<int> internalIds;
        internalIds.reserve(phonemes.size());
        for (const auto& symbolNode : phonemes) {
            const std::string symbol = symbolNode.get<std::string>();
            const PhonemeId id = inventory.idForSymbol(symbol);
            if (id == kInvalidPhonemeId) {
                throw std::runtime_error(
                    "OnnxAcousticModel: acoustic token map references unknown phoneme " +
                    symbol);
            }
            internalIds.push_back(id);
        }

        for (const auto& tokenNode : entry.at("tokenIds")) {
            const int tokenId = tokenNode.get<int>();
            if (tokenId < 0 || tokenId >= out.modelVocabSize) {
                throw std::runtime_error(
                    "OnnxAcousticModel: acoustic token id out of range");
            }
            if (tokenId == out.blankTokenId) {
                throw std::runtime_error(
                    "OnnxAcousticModel: blank token must not map to a phoneme");
            }
            if (assigned[static_cast<size_t>(tokenId)]) {
                throw std::runtime_error(
                    "OnnxAcousticModel: duplicate acoustic token mapping");
            }
            assigned[static_cast<size_t>(tokenId)] = true;
            out.tokenToInternal[static_cast<size_t>(tokenId)] = internalIds;
        }
    }

    return out;
}

// Small, dependency-free band-limited resampler. The quality gate is evaluated
// before inference, so this only converts already-accepted speech to the model
// rate. A windowed sinc prevents the aliasing that direct 48 kHz -> 16 kHz
// sample dropping would introduce.
std::vector<float> resampleTo16k(const PcmBuffer& audio) {
    if (audio.sampleRateHz <= 0) {
        throw std::runtime_error("OnnxAcousticModel: invalid sample rate");
    }
    if (audio.samples.empty()) return {};
    for (float sample : audio.samples) {
        if (!std::isfinite(sample)) {
            throw std::runtime_error(
                "OnnxAcousticModel: non-finite PCM sample");
        }
    }
    if (audio.sampleRateHz == kModelSampleRate) return audio.samples;

    const double srcRate = static_cast<double>(audio.sampleRateHz);
    const double dstRate = static_cast<double>(kModelSampleRate);
    const size_t outputCount = std::max<size_t>(
        1, static_cast<size_t>(std::llround(
               static_cast<double>(audio.samples.size()) * dstRate / srcRate)));

    constexpr int radius = 12;
    constexpr double pi = 3.14159265358979323846;
    const double cutoff = std::min(1.0, dstRate / srcRate);

    std::vector<float> out(outputCount);
    for (size_t i = 0; i < outputCount; ++i) {
        const double center = static_cast<double>(i) * srcRate / dstRate;
        const int base = static_cast<int>(std::floor(center));
        double weighted = 0.0;
        double weightSum = 0.0;

        for (int tap = base - radius + 1; tap <= base + radius; ++tap) {
            if (tap < 0 || static_cast<size_t>(tap) >= audio.samples.size()) {
                continue;
            }
            const double distance = center - static_cast<double>(tap);
            if (std::abs(distance) >= radius) continue;

            const double x = pi * distance * cutoff;
            const double sinc = std::abs(x) < 1.0e-12 ? 1.0 : std::sin(x) / x;
            const double window =
                0.5 + 0.5 * std::cos(pi * distance / static_cast<double>(radius));
            const double weight = cutoff * sinc * window;
            weighted += static_cast<double>(audio.samples[static_cast<size_t>(tap)]) *
                        weight;
            weightSum += weight;
        }

        if (std::abs(weightSum) < 1.0e-12) {
            const size_t nearest = std::min(
                audio.samples.size() - 1,
                static_cast<size_t>(std::llround(center)));
            out[i] = audio.samples[nearest];
        } else {
            out[i] = static_cast<float>(weighted / weightSum);
        }
    }
    return out;
}

void normalizeLikeWav2Vec2(std::vector<float>& samples) {
    if (samples.empty()) return;

    double mean = 0.0;
    for (float v : samples) mean += v;
    mean /= static_cast<double>(samples.size());

    double variance = 0.0;
    for (float v : samples) {
        const double d = static_cast<double>(v) - mean;
        variance += d * d;
    }
    variance /= static_cast<double>(samples.size());

    const double denom = std::sqrt(variance + 1.0e-7);
    for (float& v : samples) {
        v = static_cast<float>((static_cast<double>(v) - mean) / denom);
    }
}

} // namespace

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME

struct OnnxAcousticModel::Impl {
    explicit Impl(const std::string& modelPath, TokenMapping mapping)
        : env(ORT_LOGGING_LEVEL_WARNING, "PronunciationEngine"),
          mapping(std::move(mapping)) {

        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        options.SetIntraOpNumThreads(2);
        options.SetInterOpNumThreads(1);

#if defined(__APPLE__)
        // Core ML is opportunistic. Unsupported nodes/models remain on ORT's
        // CPU provider; if Core ML registration itself is unavailable, session
        // construction proceeds CPU-only rather than disabling scoring.
        try {
            options.AppendExecutionProvider("CoreML");
        } catch (const Ort::Exception&) {
            // CPU EP is registered by default.
        }
#endif

        session = std::make_unique<Ort::Session>(env, modelPath.c_str(), options);

        Ort::AllocatorWithDefaultOptions allocator;
        const size_t inputCount = session->GetInputCount();
        if (inputCount == 0) {
            throw std::runtime_error("OnnxAcousticModel: model has no inputs");
        }

        for (size_t i = 0; i < inputCount; ++i) {
            auto name = session->GetInputNameAllocated(i, allocator);
            inputNames.emplace_back(name.get());
            inputTypes.push_back(
                session->GetInputTypeInfo(i)
                    .GetTensorTypeAndShapeInfo()
                    .GetElementType());
        }

        const size_t outputCount = session->GetOutputCount();
        if (outputCount == 0) {
            throw std::runtime_error("OnnxAcousticModel: model has no outputs");
        }
        outputIndex = 0;
        for (size_t i = 0; i < outputCount; ++i) {
            auto name = session->GetOutputNameAllocated(i, allocator);
            outputNames.emplace_back(name.get());
            if (outputNames.back() == "logits") outputIndex = i;
        }
    }

    Ort::Env env;
    Ort::SessionOptions options;
    std::unique_ptr<Ort::Session> session;
    TokenMapping mapping;
    std::vector<std::string> inputNames;
    std::vector<ONNXTensorElementDataType> inputTypes;
    std::vector<std::string> outputNames;
    size_t outputIndex = 0;
};

#else

// Keep the pImpl complete in engine-only builds where ONNX Runtime is
// intentionally absent; infer() remains fail-closed in those builds.
struct OnnxAcousticModel::Impl {};

#endif

OnnxAcousticModel::OnnxAcousticModel(
    std::string modelPath,
    std::string resourcesDir)
    : modelPath_(std::move(modelPath)),
      resourcesDir_(std::move(resourcesDir)) {

    TokenMapping mapping = loadTokenMapping(resourcesDir_);
    vocabularySize_ = mapping.internalVocabSize;
    blankColumn_ = mapping.internalBlankColumn;

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    impl_ = std::make_unique<Impl>(modelPath_, std::move(mapping));
#endif
}

OnnxAcousticModel::~OnnxAcousticModel() = default;
OnnxAcousticModel::OnnxAcousticModel(OnnxAcousticModel&&) noexcept = default;
OnnxAcousticModel& OnnxAcousticModel::operator=(OnnxAcousticModel&&) noexcept = default;

FrameLogProbs OnnxAcousticModel::infer(const PcmBuffer& audio) {
#ifndef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    (void)audio;
    throw std::runtime_error(
        "OnnxAcousticModel: this build does not include ONNX Runtime");
#else
    if (!impl_ || !impl_->session) {
        throw std::runtime_error("OnnxAcousticModel: ONNX Runtime session unavailable");
    }

    std::vector<float> inputSamples = resampleTo16k(audio);
    if (inputSamples.empty()) {
        throw std::runtime_error("OnnxAcousticModel: empty PCM input");
    }
    normalizeLikeWav2Vec2(inputSamples);

    const std::array<int64_t, 2> inputShape{
        1, static_cast<int64_t>(inputSamples.size())
    };
    std::vector<int64_t> attentionMask(inputSamples.size(), 1);
    Ort::MemoryInfo memoryInfo =
        Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    std::vector<Ort::Value> inputValues;
    std::vector<const char*> inputNamePtrs;
    inputValues.reserve(impl_->inputNames.size());
    inputNamePtrs.reserve(impl_->inputNames.size());

    bool suppliedAudio = false;
    for (size_t i = 0; i < impl_->inputNames.size(); ++i) {
        const auto type = impl_->inputTypes[i];
        const std::string& name = impl_->inputNames[i];

        if ((name == "input_values" || !suppliedAudio) &&
            type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            inputValues.emplace_back(Ort::Value::CreateTensor<float>(
                memoryInfo,
                inputSamples.data(),
                inputSamples.size(),
                inputShape.data(),
                inputShape.size()));
            suppliedAudio = true;
        } else if (name == "attention_mask" &&
                   type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) {
            inputValues.emplace_back(Ort::Value::CreateTensor<int64_t>(
                memoryInfo,
                attentionMask.data(),
                attentionMask.size(),
                inputShape.data(),
                inputShape.size()));
        } else {
            throw std::runtime_error(
                "OnnxAcousticModel: unsupported model input " + name);
        }
        inputNamePtrs.push_back(name.c_str());
    }
    if (!suppliedAudio) {
        throw std::runtime_error(
            "OnnxAcousticModel: no float audio input found");
    }

    const char* outputName = impl_->outputNames[impl_->outputIndex].c_str();
    auto outputs = impl_->session->Run(
        Ort::RunOptions{nullptr},
        inputNamePtrs.data(),
        inputValues.data(),
        inputValues.size(),
        &outputName,
        1);

    if (outputs.size() != 1 || !outputs[0].IsTensor()) {
        throw std::runtime_error("OnnxAcousticModel: invalid logits output");
    }

    auto info = outputs[0].GetTensorTypeAndShapeInfo();
    if (info.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
        throw std::runtime_error(
            "OnnxAcousticModel: logits output is not float32");
    }

    const std::vector<int64_t> shape = info.GetShape();
    int64_t frames = 0;
    int64_t modelVocab = 0;
    if (shape.size() == 3 && shape[0] == 1) {
        frames = shape[1];
        modelVocab = shape[2];
    } else if (shape.size() == 2) {
        frames = shape[0];
        modelVocab = shape[1];
    } else {
        throw std::runtime_error(
            "OnnxAcousticModel: unexpected logits tensor rank");
    }

    if (frames <= 0 ||
        modelVocab != impl_->mapping.modelVocabSize) {
        throw std::runtime_error(
            "OnnxAcousticModel: logits vocabulary does not match pinned token map");
    }

    const float* logits = outputs[0].GetTensorData<float>();
    FrameLogProbs result;
    result.numFrames = static_cast<int>(frames);
    result.vocabSize = impl_->mapping.internalVocabSize;
    result.blankColumn = impl_->mapping.internalBlankColumn;
    result.frameDurationSeconds =
        (static_cast<double>(inputSamples.size()) / kModelSampleRate) /
        static_cast<double>(frames);
    result.data.assign(
        static_cast<size_t>(result.numFrames) *
            static_cast<size_t>(result.vocabSize),
        kLogZero);

    for (int frame = 0; frame < result.numFrames; ++frame) {
        const float* row =
            logits + static_cast<size_t>(frame) * static_cast<size_t>(modelVocab);

        float maxLogit = -std::numeric_limits<float>::infinity();
        for (int token = 0; token < modelVocab; ++token) {
            maxLogit = std::max(maxLogit, row[token]);
        }
        if (!std::isfinite(maxLogit)) {
            throw std::runtime_error(
                "OnnxAcousticModel: non-finite logits");
        }

        double sumExp = 0.0;
        for (int token = 0; token < modelVocab; ++token) {
            if (!std::isfinite(row[token])) {
                throw std::runtime_error(
                    "OnnxAcousticModel: non-finite logits");
            }
            sumExp += std::exp(
                static_cast<double>(row[token] - maxLogit));
        }
        const float logNormalizer =
            maxLogit + static_cast<float>(std::log(sumExp));

        float* internalRow =
            result.data.data() +
            static_cast<size_t>(frame) * static_cast<size_t>(result.vocabSize);

        for (int token = 0; token < modelVocab; ++token) {
            const float tokenLogProbability = row[token] - logNormalizer;
            const auto& targets =
                impl_->mapping.tokenToInternal[static_cast<size_t>(token)];

            if (token == impl_->mapping.blankTokenId || targets.empty()) {
                internalRow[result.blankColumn] =
                    logAdd(internalRow[result.blankColumn], tokenLogProbability);
                continue;
            }

            // Composite IPA labels (for example aɪɚ) contribute evidence to
            // each constituent engine phone while preserving total mass.
            const float splitLogProbability =
                tokenLogProbability -
                static_cast<float>(std::log(
                    static_cast<double>(targets.size())));
            for (int internalId : targets) {
                internalRow[internalId] =
                    logAdd(internalRow[internalId], splitLogProbability);
            }
        }
    }

    return result;
#endif
}

int OnnxAcousticModel::vocabularySize() const {
    return vocabularySize_;
}

int OnnxAcousticModel::blankColumn() const {
    return blankColumn_;
}

} // namespace pronunciation
