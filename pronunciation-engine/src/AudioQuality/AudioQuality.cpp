#include "AudioQuality.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace pronunciation {

namespace {

constexpr float kMinLinearForDb = 1e-9f;

float linearToDbfs(float linear) {
    return 20.0f * std::log10(std::max(linear, kMinLinearForDb));
}

// Splits the buffer into ~20ms frames and returns each frame's RMS energy
// (linear, not dB) so silence/SNR estimation can reason about the envelope
// rather than a single whole-buffer average, which would hide a short loud
// burst inside an otherwise silent recording.
std::vector<float> frameRmsEnergies(const PcmBuffer& audio, int frameSamples) {
    std::vector<float> energies;
    if (frameSamples <= 0 || audio.samples.empty()) return energies;
    for (size_t start = 0; start < audio.samples.size(); start += frameSamples) {
        size_t end = std::min(audio.samples.size(), start + static_cast<size_t>(frameSamples));
        double sumSquares = 0.0;
        for (size_t i = start; i < end; ++i) {
            sumSquares += static_cast<double>(audio.samples[i]) * audio.samples[i];
        }
        double meanSquare = sumSquares / static_cast<double>(end - start);
        energies.push_back(static_cast<float>(std::sqrt(meanSquare)));
    }
    return energies;
}

} // namespace

AudioQualityGate::AudioQualityGate(AudioQualityThresholds thresholds) : thresholds_(thresholds) {}

AudioQualityResult AudioQualityGate::evaluate(const PcmBuffer& audio) const {
    AudioQualityResult result;

    if (audio.durationSeconds() < thresholds_.minDurationSeconds) {
        result.reason = AudioQualityReason::TooShort;
        return result;
    }
    if (audio.samples.empty()) {
        result.reason = AudioQualityReason::Silence;
        return result;
    }

    double sumSquares = 0.0;
    float peak = 0.0f;
    size_t clippedCount = 0;
    for (float sample : audio.samples) {
        sumSquares += static_cast<double>(sample) * sample;
        float magnitude = std::fabs(sample);
        peak = std::max(peak, magnitude);
        if (magnitude >= thresholds_.clippingAmplitude) ++clippedCount;
    }
    float overallRms = static_cast<float>(std::sqrt(sumSquares / audio.samples.size()));
    result.rmsDbfs = linearToDbfs(overallRms);
    result.peakAmplitude = peak;
    result.clippingRatio = static_cast<float>(clippedCount) / static_cast<float>(audio.samples.size());

    int frameSamples = std::max(1, static_cast<int>(0.02 * audio.sampleRateHz));
    std::vector<float> frameEnergies = frameRmsEnergies(audio, frameSamples);
    std::vector<float> sorted = frameEnergies;
    std::sort(sorted.begin(), sorted.end());
    // Noise floor: median of the quietest 20% of frames. Signal level: 90th
    // percentile frame, so a handful of loud voiced frames are enough to
    // establish speech is present even in a mostly-quiet recording.
    size_t noiseSampleCount = std::max<size_t>(1, sorted.size() / 5);
    double noiseSum = 0.0;
    for (size_t i = 0; i < noiseSampleCount; ++i) noiseSum += sorted[i];
    float noiseFloor = static_cast<float>(noiseSum / noiseSampleCount);
    size_t signalIndex = static_cast<size_t>(0.9 * (sorted.size() - 1));
    float signalLevel = sorted.empty() ? overallRms : sorted[signalIndex];
    result.estimatedSnrDb = linearToDbfs(signalLevel) - linearToDbfs(noiseFloor);

    if (result.rmsDbfs < thresholds_.silenceRmsDbfs) {
        result.reason = AudioQualityReason::Silence;
        return result;
    }
    if (result.clippingRatio > thresholds_.maxClippingRatio) {
        result.reason = AudioQualityReason::Clipping;
        return result;
    }
    if (result.estimatedSnrDb < thresholds_.minSnrDb) {
        result.reason = AudioQualityReason::LowSignalToNoise;
        return result;
    }

    result.reason = AudioQualityReason::Ok;
    result.passesGate = true;
    return result;
}

} // namespace pronunciation
