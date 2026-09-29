#pragma once

#include "../Types.h"

namespace pronunciation {

struct AudioQualityThresholds {
    double minDurationSeconds = 0.3;
    float silenceRmsDbfs = -45.0f;   // below this, treat the buffer as silence
    float clippingAmplitude = 0.98f; // samples at/above this magnitude count as clipped
    float maxClippingRatio = 0.01f;  // fraction of samples allowed to be clipped
    float minSnrDb = 8.0f;
};

// Gate #2 of the locked engine pipeline (docs/product-specification.md):
// "reject silence, clipping, incomplete speech and unusable signal-to-noise
// conditions before scoring." Runs before any acoustic-model inference so a
// bad recording never reaches (and never burns) a scored attempt.
class AudioQualityGate {
public:
    explicit AudioQualityGate(AudioQualityThresholds thresholds = {});

    AudioQualityResult evaluate(const PcmBuffer& audio) const;

private:
    AudioQualityThresholds thresholds_;
};

} // namespace pronunciation
