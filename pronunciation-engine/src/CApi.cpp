#include "../include/PronunciationEngine.h"

#include <cstring>
#include <exception>
#include <memory>
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

char* copyErrorMessage(const std::string& what) {
    char* buffer = static_cast<char*>(std::malloc(what.size() + 1));
    if (buffer) std::memcpy(buffer, what.c_str(), what.size() + 1);
    return buffer;
}

pe_audio_quality_reason toCReason(pronunciation::AudioQualityReason reason) {
    switch (reason) {
        case pronunciation::AudioQualityReason::Ok: return PE_AUDIO_OK;
        case pronunciation::AudioQualityReason::TooShort: return PE_AUDIO_TOO_SHORT;
        case pronunciation::AudioQualityReason::Silence: return PE_AUDIO_SILENCE;
        case pronunciation::AudioQualityReason::Clipping: return PE_AUDIO_CLIPPING;
        case pronunciation::AudioQualityReason::LowSignalToNoise: return PE_AUDIO_LOW_SNR;
    }
    return PE_AUDIO_OK;
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

} // namespace

pe_engine* pe_engine_create(const char* resources_dir, const char* model_path, char** out_error_message) {
    try {
        auto model = std::make_unique<pronunciation::OnnxAcousticModel>(model_path);
        auto* engine = new pe_engine{pronunciation::PronunciationEngine(resources_dir, std::move(model))};
        return engine;
    } catch (const std::exception& e) {
        if (out_error_message) *out_error_message = copyErrorMessage(e.what());
        return nullptr;
    }
}

pe_engine* pe_engine_create_with_mock_model(const char* resources_dir, const pe_scripted_frame* script, int script_count,
                                             int frames_per_phoneme, char** out_error_message) {
    try {
        // Vocabulary size / blank column are derived after the content pack
        // loads (they depend on the phoneme inventory), so the mock model
        // is built in two steps: load the inventory once to size it, then
        // hand the same inventory size to both the model and the engine via
        // a throwaway load. This mirrors how the production ONNX backend's
        // vocabulary is fixed by the bundled model's own output layer.
        pronunciation::PhonemeInventory inventory =
            pronunciation::PhonemeInventory::loadFromFile(std::string(resources_dir) + "/phonemes.json");
        int vocabSize = static_cast<int>(inventory.size()) + 1;
        int blankColumn = static_cast<int>(inventory.size());

        auto model = std::make_unique<pronunciation::MockAcousticModel>(vocabSize, blankColumn);
        std::vector<pronunciation::ScriptedFrame> sequence;
        sequence.reserve(static_cast<size_t>(script_count));
        for (int i = 0; i < script_count; ++i) {
            sequence.push_back({script[i].dominant_phoneme_id, script[i].dominant_probability});
        }
        model->setScriptFromSequence(sequence, frames_per_phoneme);

        auto* engine = new pe_engine{pronunciation::PronunciationEngine(resources_dir, std::move(model))};
        return engine;
    } catch (const std::exception& e) {
        if (out_error_message) *out_error_message = copyErrorMessage(e.what());
        return nullptr;
    }
}

void pe_engine_destroy(pe_engine* engine) {
    delete engine;
}

pe_engine_result* pe_engine_process(pe_engine* engine, const float* pcm_samples, int sample_count, int sample_rate_hz,
                                     const char* exercise_id, char** out_error_message) {
    try {
        pronunciation::PcmBuffer audio;
        audio.samples.assign(pcm_samples, pcm_samples + sample_count);
        audio.sampleRateHz = sample_rate_hz;
        auto* result = new pe_engine_result{engine->impl.process(audio, exercise_id)};
        return result;
    } catch (const std::exception& e) {
        if (out_error_message) *out_error_message = copyErrorMessage(e.what());
        return nullptr;
    }
}

pe_audio_quality_result pe_result_audio_quality(const pe_engine_result* result) {
    const auto& q = result->impl.audioQuality;
    return pe_audio_quality_result{q.passesGate ? 1 : 0, toCReason(q.reason), q.rmsDbfs, q.peakAmplitude, q.clippingRatio,
                                    q.estimatedSnrDb};
}

pe_diagnosis pe_result_diagnosis(const pe_engine_result* result) {
    const auto& d = result->impl.diagnosis;
    pe_diagnosis out;
    out.outcome = toCOutcome(d.outcome);
    out.confidence = toCConfidence(d.confidence);
    out.error_pattern_id = d.errorPatternId.empty() ? nullptr : d.errorPatternId.c_str();
    out.expected_phoneme_id = d.expectedPhonemeId;
    out.produced_phoneme_id = d.producedPhonemeId;
    out.expected_index = d.expectedIndex;
    out.error_confirmed_by_history = d.errorConfirmedByHistory ? 1 : 0;
    return out;
}

int pe_result_phoneme_count(const pe_engine_result* result) {
    return static_cast<int>(result->impl.phonemeScores.size());
}

pe_phoneme_verdict pe_result_phoneme_verdict_at(const pe_engine_result* result, int index) {
    return toCVerdict(result->impl.phonemeScores.at(static_cast<size_t>(index)).verdict);
}

float pe_result_phoneme_gop_score_at(const pe_engine_result* result, int index) {
    return result->impl.phonemeScores.at(static_cast<size_t>(index)).gopScore;
}

void pe_engine_result_free(pe_engine_result* result) {
    delete result;
}

void pe_free_error_message(char* message) {
    std::free(message);
}
