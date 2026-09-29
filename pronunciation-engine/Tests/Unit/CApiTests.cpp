#include "../../include/PronunciationEngine.h"
#include "TestFramework.h"

#include <cmath>
#include <vector>

namespace {

std::vector<float> makeSpeechLikeTone(double seconds, int sampleRateHz = 16000) {
    std::vector<float> samples(static_cast<size_t>(seconds * sampleRateHz));
    for (size_t i = 0; i < samples.size(); ++i) samples[i] = 0.5f * std::sin(2.0 * M_PI * 220.0 * i / sampleRateHz);
    size_t noiseFloorSamples = samples.size() / 5;
    for (size_t i = 0; i < noiseFloorSamples; ++i) samples[i] *= 0.02f;
    return samples;
}

} // namespace

// Exercises the narrow C boundary Swift will link against: no C++ type
// crosses it, and no exception should ever escape it (pe_engine_create /
// pe_engine_process report failure through out_error_message instead).
TEST(c_api_round_trips_a_correct_pronunciation) {
    const char* resourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;
    // K=24, AE=1, T=33 in Resources/phonemes.json's declaration order.
    pe_scripted_frame script[] = {{24, 0.9f}, {1, 0.9f}, {33, 0.9f}};

    char* error = nullptr;
    pe_engine* engine = pe_engine_create_with_mock_model(resourcesDir, script, 3, 5, &error);
    REQUIRE(engine != nullptr);
    REQUIRE(error == nullptr);

    std::vector<float> pcm = makeSpeechLikeTone(1.0);
    pe_engine_result* result = pe_engine_process(engine, pcm.data(), static_cast<int>(pcm.size()), 16000, "word:cat", &error);
    REQUIRE(result != nullptr);
    REQUIRE(error == nullptr);

    pe_audio_quality_result audioQuality = pe_result_audio_quality(result);
    REQUIRE(audioQuality.passes_gate == 1);

    pe_diagnosis diagnosis = pe_result_diagnosis(result);
    REQUIRE(diagnosis.outcome == PE_OUTCOME_PASS);
    REQUIRE(pe_result_phoneme_count(result) == 3);
    for (int i = 0; i < pe_result_phoneme_count(result); ++i) {
        REQUIRE(pe_result_phoneme_verdict_at(result, i) == PE_VERDICT_CORRECT);
    }

    pe_engine_result_free(result);
    pe_engine_destroy(engine);
}

TEST(c_api_engine_only_build_reports_missing_onnx_runtime_without_crashing) {
    const char* resourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;
    char* error = nullptr;
    pe_engine* engine = pe_engine_create(resourcesDir, "/tmp/does-not-matter.onnx", &error);
    REQUIRE(engine != nullptr); // token/content mapping still validates without ORT
    REQUIRE(error == nullptr);

    std::vector<float> pcm = makeSpeechLikeTone(1.0);
    error = nullptr;
    pe_engine_result* result = pe_engine_process(engine, pcm.data(), static_cast<int>(pcm.size()), 16000, "word:cat", &error);
    REQUIRE(result == nullptr);
    REQUIRE(error != nullptr);
    pe_free_error_message(error);
    pe_engine_destroy(engine);
}

TEST(c_api_audio_quality_gate_does_not_require_model_construction) {
    std::vector<float> silence(16000, 0.0f);
    pe_audio_quality_result silent = pe_audio_quality_evaluate(
        silence.data(), static_cast<int>(silence.size()), 16000);
    REQUIRE(silent.passes_gate == 0);
    REQUIRE(silent.reason == PE_AUDIO_SILENCE);

    std::vector<float> shortClip(100, 0.2f);
    pe_audio_quality_result tooShort = pe_audio_quality_evaluate(
        shortClip.data(), static_cast<int>(shortClip.size()), 16000);
    REQUIRE(tooShort.passes_gate == 0);
    REQUIRE(tooShort.reason == PE_AUDIO_TOO_SHORT);
}
