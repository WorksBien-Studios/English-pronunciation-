#ifndef PRONUNCIATION_ENGINE_H
#define PRONUNCIATION_ENGINE_H

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
    const char* error_pattern_id;
    int expected_phoneme_id;
    int produced_phoneme_id;
    int expected_index;
    int error_confirmed_by_history;
} pe_diagnosis;

typedef struct {
    int dominant_phoneme_id;
    float dominant_probability;
} pe_scripted_frame;

// All creation/processing functions trap C++ exceptions and report failure
// through out_error_message. On success, *out_error_message is cleared to NULL.
// Runs the shared audio-quality gate without constructing/loading the acoustic
// model. This keeps silence/clipping/too-short feedback available even when
// model initialization fails.
pe_audio_quality_result pe_audio_quality_evaluate(
    const float* pcm_samples,
    int sample_count,
    int sample_rate_hz);

pe_engine* pe_engine_create(
    const char* resources_dir,
    const char* model_path,
    char** out_error_message);

pe_engine* pe_engine_create_with_mock_model(
    const char* resources_dir,
    const pe_scripted_frame* script,
    int script_count,
    int frames_per_phoneme,
    char** out_error_message);

void pe_engine_destroy(pe_engine* engine);

pe_engine_result* pe_engine_process(
    pe_engine* engine,
    const float* pcm_samples,
    int sample_count,
    int sample_rate_hz,
    const char* exercise_id,
    char** out_error_message);

// Accessors are total functions: NULL result pointers and invalid indices
// never throw across the C boundary. They return conservative sentinels:
// bad/NULL audio => passes_gate=0, diagnosis => Retry/Low,
// invalid verdict index => LOW_CONFIDENCE, invalid GOP index => 0.
pe_audio_quality_result pe_result_audio_quality(const pe_engine_result* result);
pe_diagnosis pe_result_diagnosis(const pe_engine_result* result);
int pe_result_phoneme_count(const pe_engine_result* result);
pe_phoneme_verdict pe_result_phoneme_verdict_at(
    const pe_engine_result* result,
    int index);
float pe_result_phoneme_gop_score_at(
    const pe_engine_result* result,
    int index);

void pe_engine_result_free(pe_engine_result* result);
void pe_free_error_message(char* message);

#ifdef __cplusplus
}
#endif

#endif // PRONUNCIATION_ENGINE_H
