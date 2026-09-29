#include "../../include/PronunciationEngine.h"

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s RESOURCES_DIR MODEL_PATH\n", argv[0]);
        return 2;
    }

    char* error = nullptr;
    pe_engine* engine = pe_engine_create(argv[1], argv[2], &error);
    if (!engine) {
        std::fprintf(stderr, "engine creation failed: %s\n", error ? error : "unknown");
        pe_free_error_message(error);
        return 1;
    }

    constexpr int sampleRate = 16000;
    std::vector<float> audio(sampleRate);
    for (size_t index = 0; index < audio.size(); ++index) {
        const float envelope = index < audio.size() / 5 ? 0.01f : 0.45f;
        audio[index] = envelope * std::sin(
            2.0 * 3.14159265358979323846 * 220.0 *
            static_cast<double>(index) / sampleRate);
    }

    pe_engine_result* result = pe_engine_process(
        engine,
        audio.data(),
        static_cast<int>(audio.size()),
        sampleRate,
        "word:right",
        &error);
    if (!result) {
        std::fprintf(stderr, "inference failed: %s\n", error ? error : "unknown");
        pe_free_error_message(error);
        pe_engine_destroy(engine);
        return 1;
    }

    const pe_audio_quality_result quality = pe_result_audio_quality(result);
    std::printf(
        "ONNX inference completed: audio_pass=%d phonemes=%d outcome=%d\n",
        quality.passes_gate,
        pe_result_phoneme_count(result),
        static_cast<int>(pe_result_diagnosis(result).outcome));
    pe_engine_result_free(result);
    pe_engine_destroy(engine);
    return quality.passes_gate ? 0 : 1;
}
