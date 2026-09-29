#include "Prosody.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#endif

namespace pronunciation {

namespace {

constexpr float kMinLinearForDb = 1e-9f;

// The two hot loops of the prosody stage. Apple builds use Accelerate/vDSP (single-precision, vectorised);
// every other build uses the portable double-precision loops. Everything else in this file is shared, so the
// unit tests run against both implementations (the Apple CI job runs them with vDSP).
double sumSquares(const float* samples, int count) {
    if (count <= 0) return 0.0;
#if defined(__APPLE__)
    float result = 0.0f;
    vDSP_svesq(samples, 1, &result, static_cast<vDSP_Length>(count));
    return static_cast<double>(result);
#else
    double total = 0.0;
    for (int i = 0; i < count; ++i) total += static_cast<double>(samples[i]) * samples[i];
    return total;
#endif
}

double dotProduct(const float* a, const float* b, int count) {
    if (count <= 0) return 0.0;
#if defined(__APPLE__)
    float result = 0.0f;
    vDSP_dotpr(a, 1, b, 1, &result, static_cast<vDSP_Length>(count));
    return static_cast<double>(result);
#else
    double total = 0.0;
    for (int i = 0; i < count; ++i) total += static_cast<double>(a[i]) * b[i];
    return total;
#endif
}

float rmsDb(const std::vector<float>& samples, int start, int end) {
    if (end <= start) return -120.0f;
    double energy = sumSquares(samples.data() + start, end - start);
    float rms = static_cast<float>(std::sqrt(energy / (end - start)));
    return 20.0f * std::log10(std::max(rms, kMinLinearForDb));
}

// Autocorrelation-based F0 estimate over the human-voice range (~70-400 Hz).
// Returns 0 when the segment is too short or the strongest periodicity peak
// is too weak to trust (unvoiced consonant, silence, or noise).
float estimatePitchHz(const std::vector<float>& samples, int start, int end, int sampleRateHz) {
    int n = end - start;
    int minLag = sampleRateHz / 400;
    int maxLag = sampleRateHz / 70;
    if (n < maxLag * 2 || minLag < 1) return 0.0f;

    double energy0 = sumSquares(samples.data() + start, n);
    if (energy0 < 1e-9) return 0.0f;

    int bestLag = -1;
    double bestCorrelation = 0.0;
    for (int lag = minLag; lag <= maxLag && start + lag < end; ++lag) {
        double correlation = dotProduct(samples.data() + start, samples.data() + start + lag, n - lag) / energy0;
        if (correlation > bestCorrelation) {
            bestCorrelation = correlation;
            bestLag = lag;
        }
    }

    constexpr double kVoicingThreshold = 0.3;
    if (bestLag < 0 || bestCorrelation < kVoicingThreshold) return 0.0f;
    return static_cast<float>(sampleRateHz) / static_cast<float>(bestLag);
}

float meanOf(const std::vector<float>& values) {
    if (values.empty()) return 0.0f;
    return std::accumulate(values.begin(), values.end(), 0.0f) / values.size();
}

float stddevOf(const std::vector<float>& values, float mean) {
    if (values.size() < 2) return 1.0f;
    double sumSq = 0.0;
    for (float v : values) sumSq += (v - mean) * (v - mean);
    float sd = static_cast<float>(std::sqrt(sumSq / (values.size() - 1)));
    return sd < 1e-3f ? 1.0f : sd;
}

} // namespace

ProsodyFeatures extractProsody(const PcmBuffer& audio, const ForcedAlignmentResult& alignment, const ExerciseDefinition& exercise,
                                double frameDurationSeconds) {
    ProsodyFeatures features;
    const size_t n = alignment.phonemes.size();
    features.durationsSeconds.assign(n, 0.0f);
    features.energyDb.assign(n, -120.0f);
    features.pitchHz.assign(n, 0.0f);
    features.stressProminence.assign(n, 0.0f);

    int frameSamples = std::max(1, static_cast<int>(std::lround(frameDurationSeconds * audio.sampleRateHz)));

    for (size_t i = 0; i < n; ++i) {
        const auto& aligned = alignment.phonemes[i];
        if (!aligned.hasFrames()) continue;

        int sampleStart = std::min<int>(static_cast<int>(audio.samples.size()), aligned.startFrame * frameSamples);
        int sampleEnd = std::min<int>(static_cast<int>(audio.samples.size()), (aligned.endFrame + 1) * frameSamples);

        features.durationsSeconds[i] = (aligned.endFrame - aligned.startFrame + 1) * static_cast<float>(frameDurationSeconds);
        features.energyDb[i] = rmsDb(audio.samples, sampleStart, sampleEnd);

        if (i < exercise.expectedPhonemes.size() && exercise.expectedPhonemes[i].isVowel) {
            features.pitchHz[i] = estimatePitchHz(audio.samples, sampleStart, sampleEnd, audio.sampleRateHz);
        }
    }

    std::vector<size_t> vowelIndices;
    for (size_t i = 0; i < n && i < exercise.expectedPhonemes.size(); ++i) {
        if (exercise.expectedPhonemes[i].isVowel && alignment.phonemes[i].hasFrames()) vowelIndices.push_back(i);
    }

    if (vowelIndices.size() >= 2) {
        std::vector<float> durations, energies, pitches;
        for (size_t i : vowelIndices) {
            durations.push_back(features.durationsSeconds[i]);
            energies.push_back(features.energyDb[i]);
            pitches.push_back(features.pitchHz[i]);
        }
        float meanDur = meanOf(durations), sdDur = stddevOf(durations, meanDur);
        float meanEnergy = meanOf(energies), sdEnergy = stddevOf(energies, meanEnergy);
        float meanPitch = meanOf(pitches), sdPitch = stddevOf(pitches, meanPitch);

        for (size_t i : vowelIndices) {
            float durationZ = (features.durationsSeconds[i] - meanDur) / sdDur;
            float energyZ = (features.energyDb[i] - meanEnergy) / sdEnergy;
            float pitchZ = (features.pitchHz[i] - meanPitch) / sdPitch;
            features.stressProminence[i] = std::clamp(0.4f * durationZ + 0.4f * energyZ + 0.2f * pitchZ, -3.0f, 3.0f);
        }
    }

    double totalDuration = audio.durationSeconds();
    features.speakingRateSyllablesPerSecond =
        totalDuration > 0.0 ? static_cast<float>(vowelIndices.size() / totalDuration) : 0.0f;

    return features;
}

} // namespace pronunciation
