#include "../../include/PronunciationEngine.h"
#include "../../src/CTCAlignment/CTCAlignment.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace pronunciation;

namespace {

std::vector<float> makeSpeechLikeTone(double seconds, int sampleRateHz = 16000) {
    std::vector<float> samples(static_cast<size_t>(seconds * sampleRateHz));
    for (size_t i = 0; i < samples.size(); ++i) {
        samples[i] = 0.5f * std::sin(2.0 * M_PI * 220.0 * i / sampleRateHz);
    }
    size_t quiet = samples.size() / 5;
    for (size_t i = 0; i < quiet; ++i) samples[i] *= 0.02f;
    return samples;
}

pe_engine* createEngine(char** error) {
    pe_scripted_frame script[] = {{24, 0.9f}, {1, 0.9f}, {33, 0.9f}};
    return pe_engine_create_with_mock_model(
        PRONUNCIATION_ENGINE_RESOURCES_DIR, script, 3, 5, error);
}

pe_engine_result* createResult(pe_engine* engine, char** error) {
    auto pcm = makeSpeechLikeTone(1.0);
    return pe_engine_process(
        engine, pcm.data(), static_cast<int>(pcm.size()), 16000,
        "word:cat", error);
}

int expectRejected(pe_engine_result* result, char* error) {
    const bool rejected = result == nullptr && error != nullptr;
    if (result) pe_engine_result_free(result);
    if (error) pe_free_error_message(error);
    return rejected ? 0 : 2;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) return 64;
    const std::string test = argv[1];

    if (test == "null_engine_process") {
        char* error = nullptr;
        auto pcm = makeSpeechLikeTone(1.0);
        pe_engine_result* result = pe_engine_process(
            nullptr, pcm.data(), static_cast<int>(pcm.size()), 16000,
            "word:cat", &error);
        return expectRejected(result, error);
    }

    if (test == "null_pcm_positive_count") {
        char* error = nullptr;
        pe_engine* engine = createEngine(&error);
        if (!engine) return 65;
        pe_engine_result* result =
            pe_engine_process(engine, nullptr, 16000, 16000, "word:cat", &error);
        int rc = expectRejected(result, error);
        pe_engine_destroy(engine);
        return rc;
    }

    if (test == "null_script_positive_count") {
        char* error = nullptr;
        pe_engine* engine = pe_engine_create_with_mock_model(
            PRONUNCIATION_ENGINE_RESOURCES_DIR, nullptr, 1, 5, &error);
        const bool rejected = engine == nullptr && error != nullptr;
        if (engine) pe_engine_destroy(engine);
        if (error) pe_free_error_message(error);
        return rejected ? 0 : 2;
    }

    if (test == "negative_sample_count") {
        char* error = nullptr;
        pe_engine* engine = createEngine(&error);
        if (!engine) return 65;
        float sample = 0.2f;
        pe_engine_result* result =
            pe_engine_process(engine, &sample, -1, 16000, "word:cat", &error);
        int rc = expectRejected(result, error);
        pe_engine_destroy(engine);
        return rc;
    }

    if (test == "null_exercise_id") {
        char* error = nullptr;
        pe_engine* engine = createEngine(&error);
        if (!engine) return 65;
        auto pcm = makeSpeechLikeTone(1.0);
        pe_engine_result* result =
            pe_engine_process(
                engine, pcm.data(), static_cast<int>(pcm.size()), 16000,
                nullptr, &error);
        int rc = expectRejected(result, error);
        pe_engine_destroy(engine);
        return rc;
    }

    if (test == "negative_result_index") {
        char* error = nullptr;
        pe_engine* engine = createEngine(&error);
        if (!engine) return 65;
        pe_engine_result* result = createResult(engine, &error);
        if (!result) return 66;

        pe_phoneme_verdict verdict =
            pe_result_phoneme_verdict_at(result, -1);

        pe_engine_result_free(result);
        pe_engine_destroy(engine);
        return verdict == PE_VERDICT_LOW_CONFIDENCE ? 0 : 2;
    }

    if (test == "past_end_result_index") {
        char* error = nullptr;
        pe_engine* engine = createEngine(&error);
        if (!engine) return 65;
        pe_engine_result* result = createResult(engine, &error);
        if (!result) return 66;

        const int count = pe_result_phoneme_count(result);
        const float gop = pe_result_phoneme_gop_score_at(result, count);

        pe_engine_result_free(result);
        pe_engine_destroy(engine);
        return gop == 0.0f ? 0 : 2;
    }

    if (test == "null_result_accessors") {
        const pe_audio_quality_result quality = pe_result_audio_quality(nullptr);
        const pe_diagnosis diagnosis = pe_result_diagnosis(nullptr);
        const int count = pe_result_phoneme_count(nullptr);
        const pe_phoneme_verdict verdict =
            pe_result_phoneme_verdict_at(nullptr, 0);
        const float gop = pe_result_phoneme_gop_score_at(nullptr, 0);

        return (!quality.passes_gate &&
                diagnosis.outcome == PE_OUTCOME_RETRY &&
                diagnosis.confidence == PE_CONFIDENCE_LOW &&
                count == 0 &&
                verdict == PE_VERDICT_LOW_CONFIDENCE &&
                gop == 0.0f)
                   ? 0
                   : 2;
    }

    if (test == "null_create_paths") {
        char* error1 = nullptr;
        pe_engine* first = pe_engine_create(nullptr, "/tmp/model.onnx", &error1);
        const bool firstRejected = first == nullptr && error1 != nullptr;
        if (first) pe_engine_destroy(first);
        if (error1) pe_free_error_message(error1);

        char* error2 = nullptr;
        pe_engine* second =
            pe_engine_create(PRONUNCIATION_ENGINE_RESOURCES_DIR, nullptr, &error2);
        const bool secondRejected = second == nullptr && error2 != nullptr;
        if (second) pe_engine_destroy(second);
        if (error2) pe_free_error_message(error2);

        return firstRejected && secondRejected ? 0 : 2;
    }

    if (test == "malformed_logit_buffer") {
        FrameLogProbs logits;
        logits.numFrames = 5;
        logits.vocabSize = 4;
        logits.blankColumn = 3;
        logits.data = {-0.1f};
        auto result = forceAlign(logits, {0});
        return result.succeeded ? 2 : 0;
    }

    if (test == "out_of_range_expected_phoneme") {
        FrameLogProbs logits;
        logits.numFrames = 5;
        logits.vocabSize = 4;
        logits.blankColumn = 3;
        logits.data.assign(20, -1.0f);
        auto result = forceAlign(logits, {99});
        return result.succeeded ? 2 : 0;
    }

    std::fprintf(stderr, "unknown probe: %s\n", test.c_str());
    return 64;
}
