#pragma once

#include "../Types.h"

namespace pronunciation {

// Pipeline step 7 (docs/product-specification.md#model-strategy): "calculate
// duration, energy, pitch, stress and rhythm features with Accelerate/vDSP."
//
// This is the portable reference implementation (autocorrelation pitch
// tracking, RMS-dB energy, plain duration arithmetic) used for engine
// development, tests, and non-Apple builds. It is written as a single pure
// function over PCM + alignment so an Accelerate/vDSP-backed implementation
// can be substituted behind the same signature for the iOS target without
// touching CTCAlignment, PhonemeScoring, or DecisionRules, none of which
// depend on how these features were computed.
ProsodyFeatures extractProsody(const PcmBuffer& audio, const ForcedAlignmentResult& alignment, const ExerciseDefinition& exercise,
                                double frameDurationSeconds);

} // namespace pronunciation
