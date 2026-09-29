#ifndef PRONUNCIATION_ENGINE_H
#define PRONUNCIATION_ENGINE_H

// Narrow C API over the C++17 pronunciation core, per
// docs/product-specification.md: "Shared pronunciation core: C++17 static
// library exposed through a narrow C/Swift wrapper." Swift owns
// microphone/session integration (AVFoundation) and maps this typed result
// into app state; this header is the entire surface Swift links against —
// no C++ types cross the boundary, and no exception ever crosses it either
// (every function below reports failure through out_error_message instead).
//
// Deliberately narrow: only what an assessment/practice UI needs to render
// (audio-quality reason, the fused diagnosis, and per-phoneme verdicts for
// a detail view) is exposed. Internal pipeline artifacts such as raw
// acoustic-model logits or the CTC alignment lattice stay inside the C++
// layer.

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pe_engine pe_engine;
typedef struct pe_engine_result pe_engine_result;

typedef enum {
    PE_AUDIO_OK = 0,
    PE_AUDIO_TOO_SHORT = 1,
    PE_AUDIO_SILENCE = 2,
    PE_AUDIO_CLIPPING = 3,
    PE_AUDIO_LOW_SNR = 4,
} pe_audio_quality_reason;

typedef struct {
    int passes_gate;
    pe_audio_quality_reason reason;
    float rms_dbfs;
    float peak_amplitude;
    float clipping_ratio;
    float estimated_snr_db;
} pe_audio_quality_result;

typedef enum {
    PE_VERDICT_CORRECT = 0,
    PE_VERDICT_SUBSTITUTED = 1,
    PE_VERDICT_INSERTED = 2,
    PE_VERDICT_DELETED = 3,
    PE_VERDICT_LOW_CONFIDENCE = 4,
} pe_phoneme_verdict;

typedef enum {
    PE_CONFIDENCE_LOW = 0,
    PE_CONFIDENCE_MEDIUM = 1,
    PE_CONFIDENCE_HIGH = 2,
} pe_confidence_level;

typedef enum {
    PE_OUTCOME_PASS = 0,
    PE_OUTCOME_RETRY = 1,
    PE_OUTCOME_SPECIFIC_ERROR = 2,
} pe_diagnosis_outcome;

typedef struct {
    pe_diagnosis_outcome outcome;
    pe_confidence_level confidence;
    const char* error_pattern_id; // NULL if no specific pattern; valid until pe_engine_result_free
    int expected_phoneme_id;
    int produced_phoneme_id; // -1 if not applicable (e.g. a deletion)
    int expected_index;      // index into the exercise's expected phoneme sequence, -1 if not applicable
    int error_confirmed_by_history; // 1 once this pattern has recurred at high confidence >= its configured threshold
} pe_diagnosis;

// Creates an engine instance backed by the production ONNX Runtime Mobile
// acoustic-model backend (model_path is the bundled quantized Wav2Vec2
// checkpoint; see model/model-manifest.json). That backend is not wired up
// yet (see src/Acoustic/OnnxAcousticModel.h) — calls to pe_engine_process
// against an engine created this way currently fail with a descriptive
// error rather than silently mis-scoring. Engine development and tests use
// pe_engine_create_with_mock_model instead.
//
// Returns NULL and sets *out_error_message (caller must free with
// pe_free_error_message) on failure, e.g. malformed content in resources_dir.
pe_engine* pe_engine_create(const char* resources_dir, const char* model_path, char** out_error_message);

// Testing/harness entry point: creates an engine whose acoustic model
// replays a scripted phoneme sequence instead of running real inference.
// script/script_count describe one scripted frame block per entry; each
// entry is repeated frames_per_phoneme times. dominant_phoneme_id of -1
// scripts a blank/silence frame.
typedef struct {
    int dominant_phoneme_id;
    float dominant_probability;
} pe_scripted_frame;

pe_engine* pe_engine_create_with_mock_model(const char* resources_dir, const pe_scripted_frame* script, int script_count,
                                             int frames_per_phoneme, char** out_error_message);

void pe_engine_destroy(pe_engine* engine);

// exercise_id must be one of "word:<id>", "minimalPair:<id>:A",
// "minimalPair:<id>:B", or "sentence:<id>" (see Resources/*.json for ids).
// pcm_samples must be mono, normalized to [-1, 1].
//
// Returns NULL and sets *out_error_message on failure (e.g. unknown
// exercise_id). Otherwise returns an owned result that must be released
// with pe_engine_result_free.
pe_engine_result* pe_engine_process(pe_engine* engine, const float* pcm_samples, int sample_count, int sample_rate_hz,
                                     const char* exercise_id, char** out_error_message);

pe_audio_quality_result pe_result_audio_quality(const pe_engine_result* result);
pe_diagnosis pe_result_diagnosis(const pe_engine_result* result);

int pe_result_phoneme_count(const pe_engine_result* result);
pe_phoneme_verdict pe_result_phoneme_verdict_at(const pe_engine_result* result, int index);
float pe_result_phoneme_gop_score_at(const pe_engine_result* result, int index);

void pe_engine_result_free(pe_engine_result* result);
void pe_free_error_message(char* message);

#ifdef __cplusplus
}
#endif

#endif // PRONUNCIATION_ENGINE_H
