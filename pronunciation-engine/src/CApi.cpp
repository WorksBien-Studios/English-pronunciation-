#include "../include/PronunciationEngine.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "Acoustic/MockAcousticModel.h"
#include "Acoustic/OnnxAcousticModel.h"
#include "Engine.h"

struct pe_engine {
    pronunciation::PronunciationEngine impl;
};

struct pe_engine_result {
    pronunciation::EngineResult impl;
};

namespace {

void clearError(char** outErrorMessage) {
    if (outErrorMessage) *outErrorMessage = nullptr;
}

char* copyErrorMessage(const std::string& what) {
    char* buffer = static_cast<char*>(std::malloc(what.size() + 1));
    if (buffer) std::memcpy(buffer, what.c_str(), what.size() + 1);
    return buffer;
}

void setError(char** outErrorMessage, const std::string& message) {
    if (outErrorMessage) *outErrorMessage = copyErrorMessage(message);
}

pe_audio_quality_reason toCReason(pronunciation::AudioQualityReason reason) {
    switch (reason) {
        case pronunciation::AudioQualityReason::Ok: return PE_AUDIO_OK;
        case pronunciation::AudioQualityReason::TooShort: return PE_AUDIO_TOO_SHORT;
        case pronunciation::AudioQualityReason::Silence: return PE_AUDIO_SILENCE;
        case pronunciation::AudioQualityReason::Clipping: return PE_AUDIO_CLIPPING;
        case pronunciation::AudioQualityReason::LowSignalToNoise: return PE_AUDIO_LOW_SNR;
    }
    return PE_AUDIO_LOW_SNR;
}

pe_phoneme_verdict toCVerdict(pronunciation::PhonemeVerdict verdict) {
    switch (verdict) {
        case pronunciation::PhonemeVerdict::Correct: return PE_VERDICT_CORRECT;
        case pronunciation::PhonemeVerdict::Substituted: return PE_VERDICT_SUBSTITUTED;
        case pronunciation::PhonemeVerdict::Inserted: return PE_VERDICT_INSERTED;
        case pronunciation::PhonemeVerdict::Deleted: return PE_VERDICT_DELETED;
        case pronunciation::PhonemeVerdict::LowConfidence: return PE_VERDICT_LOW_CONFIDENCE;
    }
    return PE_VERDICT_LOW_CONFIDENCE;
}

pe_confidence_level toCConfidence(pronunciation::ConfidenceLevel confidence) {
    switch (confidence) {
        case pronunciation::ConfidenceLevel::Low: return PE_CONFIDENCE_LOW;
        case pronunciation::ConfidenceLevel::Medium: return PE_CONFIDENCE_MEDIUM;
        case pronunciation::ConfidenceLevel::High: return PE_CONFIDENCE_HIGH;
    }
    return PE_CONFIDENCE_LOW;
}

pe_diagnosis_outcome toCOutcome(pronunciation::DiagnosisOutcome outcome) {
    switch (outcome) {
        case pronunciation::DiagnosisOutcome::Pass: return PE_OUTCOME_PASS;
        case pronunciation::DiagnosisOutcome::Retry: return PE_OUTCOME_RETRY;
        case pronunciation::DiagnosisOutcome::SpecificError: return PE_OUTCOME_SPECIFIC_ERROR;
    }
    return PE_OUTCOME_RETRY;
}

pe_diagnosis safeRetryDiagnosis() {
    pe_diagnosis out{};
    out.outcome = PE_OUTCOME_RETRY;
    out.confidence = PE_CONFIDENCE_LOW;
    out.error_pattern_id = nullptr;
    out.expected_phoneme_id = -2;
    out.produced_phoneme_id = -2;
    out.expected_index = -1;
    out.error_confirmed_by_history = 0;
    return out;
}

} // namespace

pe_engine* pe_engine_create(
    const char* resources_dir,
    const char* model_path,
    char** out_error_message) {

    clearError(out_error_message);
    if (!resources_dir || !model_path) {
        setError(out_error_message, "pe_engine_create: resources_dir and model_path must not be NULL");
        return nullptr;
    }

    try {
        auto model = std::make_unique<pronunciation::OnnxAcousticModel>(model_path);
        return new pe_engine{
            pronunciation::PronunciationEngine(resources_dir, std::move(model))
        };
    } catch (const std::exception& e) {
        setError(out_error_message, e.what());
    } catch (...) {
        setError(out_error_message, "pe_engine_create: unknown engine construction failure");
    }
    return nullptr;
}

pe_engine* pe_engine_create_with_mock_model(
    const char* resources_dir,
    const pe_scripted_frame* script,
    int script_count,
    int frames_per_phoneme,
    char** out_error_message) {

    clearError(out_error_message);
    if (!resources_dir) {
        setError(out_error_message, "pe_engine_create_with_mock_model: resources_dir must not be NULL");
        return nullptr;
    }
    if (script_count < 0 || frames_per_phoneme <= 0) {
        setError(out_error_message, "pe_engine_create_with_mock_model: invalid script_count or frames_per_phoneme");
        return nullptr;
    }
    if (script_count > 0 && !script) {
        setError(out_error_message, "pe_engine_create_with_mock_model: script must not be NULL when script_count > 0");
        return nullptr;
    }

    try {
        pronunciation::PhonemeInventory inventory =
            pronunciation::PhonemeInventory::loadFromFile(
                std::string(resources_dir) + "/phonemes.json");
        int vocabSize = static_cast<int>(inventory.size()) + 1;
        int blankColumn = static_cast<int>(inventory.size());

        auto model = std::make_unique<pronunciation::MockAcousticModel>(
            vocabSize, blankColumn);
        std::vector<pronunciation::ScriptedFrame> sequence;
        sequence.reserve(static_cast<size_t>(script_count));

        for (int i = 0; i < script_count; ++i) {
            const int id = script[i].dominant_phoneme_id;
            const float probability = script[i].dominant_probability;
            if ((id < 0 && id != pronunciation::kBlankPhonemeId) ||
                id >= static_cast<int>(inventory.size()) ||
                !std::isfinite(probability) ||
                probability < 0.0f || probability > 1.0f) {
                setError(out_error_message,
                         "pe_engine_create_with_mock_model: invalid scripted frame");
                return nullptr;
            }
            sequence.push_back({id, probability});
        }

        model->setScriptFromSequence(sequence, frames_per_phoneme);
        return new pe_engine{
            pronunciation::PronunciationEngine(resources_dir, std::move(model))
        };
    } catch (const std::exception& e) {
        setError(out_error_message, e.what());
    } catch (...) {
        setError(out_error_message,
                 "pe_engine_create_with_mock_model: unknown engine construction failure");
    }
    return nullptr;
}

void pe_engine_destroy(pe_engine* engine) {
    delete engine;
}

pe_engine_result* pe_engine_process(
    pe_engine* engine,
    const float* pcm_samples,
    int sample_count,
    int sample_rate_hz,
    const char* exercise_id,
    char** out_error_message) {

    clearError(out_error_message);

    if (!engine) {
        setError(out_error_message, "pe_engine_process: engine must not be NULL");
        return nullptr;
    }
    if (sample_count < 0 || sample_rate_hz <= 0) {
        setError(out_error_message, "pe_engine_process: invalid sample_count or sample_rate_hz");
        return nullptr;
    }
    if (sample_count > 0 && !pcm_samples) {
        setError(out_error_message, "pe_engine_process: pcm_samples must not be NULL when sample_count > 0");
        return nullptr;
    }
    if (!exercise_id || exercise_id[0] == '\0') {
        setError(out_error_message, "pe_engine_process: exercise_id must not be NULL or empty");
        return nullptr;
    }

    try {
        pronunciation::PcmBuffer audio;
        audio.sampleRateHz = sample_rate_hz;
        if (sample_count > 0) {
            audio.samples.assign(
                pcm_samples, pcm_samples + static_cast<size_t>(sample_count));
        }

        return new pe_engine_result{
            engine->impl.process(audio, exercise_id)
        };
    } catch (const std::exception& e) {
        setError(out_error_message, e.what());
    } catch (...) {
        setError(out_error_message, "pe_engine_process: unknown processing failure");
    }
    return nullptr;
}

pe_audio_quality_result pe_result_audio_quality(const pe_engine_result* result) {
    if (!result) {
        return pe_audio_quality_result{
            0, PE_AUDIO_LOW_SNR, -120.0f, 0.0f, 0.0f, 0.0f
        };
    }

    const auto& q = result->impl.audioQuality;
    return pe_audio_quality_result{
        q.passesGate ? 1 : 0,
        toCReason(q.reason),
        q.rmsDbfs,
        q.peakAmplitude,
        q.clippingRatio,
        q.estimatedSnrDb
    };
}

pe_diagnosis pe_result_diagnosis(const pe_engine_result* result) {
    if (!result) return safeRetryDiagnosis();

    const auto& d = result->impl.diagnosis;
    pe_diagnosis out{};
    out.outcome = toCOutcome(d.outcome);
    out.confidence = toCConfidence(d.confidence);
    out.error_pattern_id =
        d.errorPatternId.empty() ? nullptr : d.errorPatternId.c_str();
    out.expected_phoneme_id = d.expectedPhonemeId;
    out.produced_phoneme_id = d.producedPhonemeId;
    out.expected_index = d.expectedIndex;
    out.error_confirmed_by_history = d.errorConfirmedByHistory ? 1 : 0;
    return out;
}

int pe_result_phoneme_count(const pe_engine_result* result) {
    if (!result) return 0;
    return static_cast<int>(result->impl.phonemeScores.size());
}

pe_phoneme_verdict pe_result_phoneme_verdict_at(
    const pe_engine_result* result,
    int index) {

    if (!result || index < 0 ||
        static_cast<size_t>(index) >= result->impl.phonemeScores.size()) {
        return PE_VERDICT_LOW_CONFIDENCE;
    }
    return toCVerdict(
        result->impl.phonemeScores[static_cast<size_t>(index)].verdict);
}

float pe_result_phoneme_gop_score_at(
    const pe_engine_result* result,
    int index) {

    if (!result || index < 0 ||
        static_cast<size_t>(index) >= result->impl.phonemeScores.size()) {
        return 0.0f;
    }
    return result->impl.phonemeScores[static_cast<size_t>(index)].gopScore;
}

void pe_engine_result_free(pe_engine_result* result) {
    delete result;
}

void pe_free_error_message(char* message) {
    std::free(message);
}
