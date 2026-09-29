#pragma once

#include <string>
#include <unordered_map>

namespace pronunciation {

// Tracks how many times each Japanese error pattern has been observed at
// high confidence. Per docs/product-specification.md: "repeated-error
// history is updated only after the same high-confidence pattern is
// observed at least twice" — a single high-confidence attempt can still
// surface a SpecificError diagnosis immediately, but it only becomes a
// *confirmed* recurring weakness (eligible for the personalized drill
// queue) once the same key has been seen `repeatsRequired` times.
//
// This is in-memory engine state; the app layer (SwiftData) is responsible
// for persisting counts across sessions and re-seeding them at startup via
// recordHighConfidenceObservation() replays, or a future bulk-load API.
class ErrorHistory {
public:
    int recordHighConfidenceObservation(const std::string& key);
    int observationCount(const std::string& key) const;
    bool isConfirmed(const std::string& key, int repeatsRequired) const;

private:
    std::unordered_map<std::string, int> counts_;
};

} // namespace pronunciation
