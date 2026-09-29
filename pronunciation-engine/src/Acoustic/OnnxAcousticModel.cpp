#include "OnnxAcousticModel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
#include <onnxruntime_cxx_api.h>
#endif

namespace pronunciation {
namespace {

constexpr int kModelSampleRate = 16000;
constexpr int kRawVocabularySize = 392;
constexpr int kInternalPhonemeCount = 40;
constexpr int kCompactBlankColumn = kInternalPhonemeCount;
constexpr int kCompactVocabularySize = kInternalPhonemeCount + 1;
constexpr int kRawBlankToken = 0;

using RawOwnerMap = std::array<int, kRawVocabularySize>;

float logAdd(float a, float b) {
    if (!std::isfinite(a)) return b;
    if (!std::isfinite(b)) return a;
    const float high = std::max(a, b);
    const float low = std::min(a, b);
    return high + std::log1p(std::exp(low - high));
}

const RawOwnerMap& rawOwnerMap() {
    static const RawOwnerMap owners = [] {
        RawOwnerMap out{};
        out.fill(kCompactBlankColumn);

        // Internal IDs are fixed by Resources/phonemes.json. The values on the
        // right are token IDs from the locked model's 392-label tokenizer.
        // Multiple upstream realizations are merged with log-sum-exp.
        const std::array<std::vector<int>, kInternalPhonemeCount> mapped = {{
            {59, 41},  // AA: ɑ, ɑː
            {36},      // AE: æ
            {33},      // AH: ʌ
            {40, 67},  // AO: ɔ, ɔː
            {53},      // AW: aʊ
            {7, 50},   // AX: ə, ᵻ
            {37},      // AY: aɪ
            {14, 87},  // EH: ɛ, ɛː
            {43, 63},  // ER: ɚ, ɜː
            {44},      // EY: eɪ
            {17},      // IH: ɪ
            {10, 30},  // IY: i, iː
            {49},      // OW: oʊ
            {100},     // OY: ɔɪ
            {29},      // UH: ʊ
            {34, 46},  // UW: u, uː
            {26},      // B
            {66},      // CH: tʃ
            {12},      // D
            {22},      // DH: ð
            {23},      // F
            {35},      // G: ɡ
            {39},      // HH: h
            {60},      // JH: dʒ
            {11},      // K
            {8, 175},  // L: l, ɫ
            {13},      // M
            {4, 169},  // N: n, syllabic n
            {42},      // NG: ŋ
            {18},      // P
            {27, 31},  // R: ɹ, r
            {5},       // S
            {38},      // SH: ʃ
            {6},       // T
            {52},      // TH: θ
            {25},      // V
            {32},      // W
            {24},      // Y: j
            {21},      // Z
            {65},      // ZH: ʒ
        }};

        std::array<bool, kRawVocabularySize> claimed{};
        claimed.fill(false);
        claimed[kRawBlankToken] = true;

        for (int internal = 0; internal < kInternalPhonemeCount; ++internal) {
            for (const int raw : mapped[static_cast<size_t>(internal)]) {
                if (raw <= 0 || raw >= kRawVocabularySize) {
                    throw std::logic_error("OnnxAcousticModel: invalid locked tokenizer mapping");
                }
                if (claimed[static_cast<size_t>(raw)]) {
                    throw std::logic_error("OnnxAcousticModel: duplicate locked tokenizer mapping");
                }
                claimed[static_cast<size_t>(raw)] = true;
                out[static_cast<size_t>(raw)] = internal;
            }
        }
        // raw token 0 and every unclaimed/non-English/composite token remain
        // in the conservative blank/unsupported bucket.
        return out;
    }();
    return owners;
}

std::vector<float> resampleTo16k(const PcmBuffer& audio) {
    if (audio.sampleRateHz <= 0) {
        throw std::invalid_argument("OnnxAcousticModel: sample rate must be positive");
    }
    if (audio.samples.empty()) {
        throw std::invalid_argument("OnnxAcousticModel: audio is empty");
    }
    for (float sample : audio.samples) {
        if (!std::isfinite(sample)) {
            throw std::invalid_argument("OnnxAcousticModel: audio contains non-finite samples");
        }
    }

    if (audio.sampleRateHz == kModelSampleRate) return audio.samples;

    const double ratio =
        static_cast<double>(kModelSampleRate) / static_cast<double>(audio.sampleRateHz);
    const size_t outputCount = std::max<size_t>(
        1, static_cast<size_t>(std::llround(static_cast<double>(audio.samples.size()) * ratio)));

    std::vector<float> output(outputCount);
    const double sourceStep =
        static_cast<double>(audio.sampleRateHz) / static_cast<double>(kModelSampleRate);

    for (size_t i = 0; i < outputCount; ++i) {
        const double position = static_cast<double>(i) * sourceStep;
        const size_t left = std::min(
            static_cast<size_t>(position), audio.samples.size() - 1);
        const size_t right = std::min(left + 1, audio.samples.size() - 1);
        const float fraction = static_cast<float>(position - static_cast<double>(left));
        output[i] = audio.samples[left] +
                    (audio.samples[right] - audio.samples[left]) * fraction;
    }
    return output;
}

} // namespace

#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME

struct OnnxAcousticModel::Impl {
    Ort::Env env{ORT_LOGGING_LEVEL_WARNING, "EnglishPronunciationCoach"};
    Ort::SessionOptions options;
    std::unique_ptr<Ort::Session> session;
    std::string inputName;
    std::string outputName;

    explicit Impl(const std::string& modelPath) {
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        options.SetIntraOpNumThreads(2);
        options.SetInterOpNumThreads(1);

        session = std::make_unique<Ort::Session>(env, modelPath.c_str(), options);
        if (session->GetInputCount() != 1 || session->GetOutputCount() != 1) {
            throw std::runtime_error(
                "OnnxAcousticModel: locked model must expose exactly one input and one output");
        }

        Ort::AllocatorWithDefaultOptions allocator;
        auto input = session->GetInputNameAllocated(0, allocator);
        auto output = session->GetOutputNameAllocated(0, allocator);
        inputName = input.get();
        outputName = output.get();

        const auto inputInfo =
            session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
        if (inputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            throw std::runtime_error("OnnxAcousticModel: expected float32 audio input");
        }

        const auto outputInfo =
            session->GetOutputTypeInfo(0).GetTensorTypeAndShapeInfo();
        if (outputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            throw std::runtime_error("OnnxAcousticModel: expected float32 logits output");
        }
        const auto outputShape = outputInfo.GetShape();
        if (outputShape.size() != 3) {
            throw std::runtime_error("OnnxAcousticModel: expected [batch, frames, vocab] logits");
        }
        if (outputShape.back() > 0 && outputShape.back() != kRawVocabularySize) {
            throw std::runtime_error(
                "OnnxAcousticModel: model tokenizer vocabulary is not the locked 392 labels");
        }
    }
};

#else

struct OnnxAcousticModel::Impl {};

#endif

OnnxAcousticModel::OnnxAcousticModel(std::string modelPath)
    : modelPath_(std::move(modelPath)) {
    if (modelPath_.empty()) {
        throw std::invalid_argument("OnnxAcousticModel: model path is empty");
    }
#ifdef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    impl_ = std::make_unique<Impl>(modelPath_);
#else
    throw std::runtime_error(
        "OnnxAcousticModel: ONNX Runtime backend was not compiled into this build");
#endif
}

OnnxAcousticModel::~OnnxAcousticModel() = default;

FrameLogProbs OnnxAcousticModel::infer(const PcmBuffer& audio) {
#ifndef PRONUNCIATION_ENGINE_WITH_ONNXRUNTIME
    (void)audio;
    throw std::runtime_error(
        "OnnxAcousticModel: ONNX Runtime backend was not compiled into this build");
#else
    if (!impl_ || !impl_->session) {
        throw std::runtime_error("OnnxAcousticModel: inference session is unavailable");
    }

    std::vector<float> samples = resampleTo16k(audio);
    const std::array<int64_t, 2> inputShape{
        1, static_cast<int64_t>(samples.size())
    };
    Ort::MemoryInfo memoryInfo =
        Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        samples.data(),
        samples.size(),
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

    if (outputs.size() != 1 || !outputs[0].IsTensor()) {
        throw std::runtime_error("OnnxAcousticModel: inference did not return one tensor");
    }

    const auto shapeInfo = outputs[0].GetTensorTypeAndShapeInfo();
    const auto shape = shapeInfo.GetShape();
    if (shape.size() != 3 || shape[0] != 1 ||
        shape[1] <= 0 || shape[2] != kRawVocabularySize) {
        throw std::runtime_error(
            "OnnxAcousticModel: unexpected logits shape; expected [1, frames, 392]");
    }

    const int numFrames = static_cast<int>(shape[1]);
    const size_t expectedRawCount =
        static_cast<size_t>(numFrames) * kRawVocabularySize;
    if (shapeInfo.GetElementCount() != expectedRawCount) {
        throw std::runtime_error("OnnxAcousticModel: logits tensor size is inconsistent");
    }

    const float* raw = outputs[0].GetTensorData<float>();
    if (!raw) {
        throw std::runtime_error("OnnxAcousticModel: logits tensor has no data");
    }

    FrameLogProbs result;
    result.numFrames = numFrames;
    result.vocabSize = kCompactVocabularySize;
    result.blankColumn = kCompactBlankColumn;
    result.frameDurationSeconds =
        (static_cast<double>(samples.size()) / kModelSampleRate) /
        static_cast<double>(numFrames);
    result.data.resize(
        static_cast<size_t>(numFrames) * kCompactVocabularySize);

    const auto& owners = rawOwnerMap();
    for (int frame = 0; frame < numFrames; ++frame) {
        std::array<float, kCompactVocabularySize> compact{};
        compact.fill(-std::numeric_limits<float>::infinity());

        const float* row =
            raw + static_cast<size_t>(frame) * kRawVocabularySize;
        for (int rawColumn = 0; rawColumn < kRawVocabularySize; ++rawColumn) {
            const float value = row[rawColumn];
            if (!std::isfinite(value)) {
                throw std::runtime_error("OnnxAcousticModel: logits contain non-finite values");
            }
            const int owner = owners[static_cast<size_t>(rawColumn)];
            compact[static_cast<size_t>(owner)] =
                logAdd(compact[static_cast<size_t>(owner)], value);
        }

        float normalizer = -std::numeric_limits<float>::infinity();
        for (float value : compact) normalizer = logAdd(normalizer, value);
        if (!std::isfinite(normalizer)) {
            throw std::runtime_error("OnnxAcousticModel: could not normalize logits");
        }

        float* destination =
            result.data.data() + static_cast<size_t>(frame) * kCompactVocabularySize;
        for (int column = 0; column < kCompactVocabularySize; ++column) {
            destination[column] =
                compact[static_cast<size_t>(column)] - normalizer;
        }
    }

    return result;
#endif
}

int OnnxAcousticModel::vocabularySize() const {
    return kCompactVocabularySize;
}

int OnnxAcousticModel::blankColumn() const {
    return kCompactBlankColumn;
}

} // namespace pronunciation
