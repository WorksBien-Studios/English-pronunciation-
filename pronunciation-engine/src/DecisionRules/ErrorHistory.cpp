#include "ErrorHistory.h"

namespace pronunciation {

int ErrorHistory::recordHighConfidenceObservation(const std::string& key) {
    return ++counts_[key];
}

int ErrorHistory::observationCount(const std::string& key) const {
    auto it = counts_.find(key);
    return it == counts_.end() ? 0 : it->second;
}

bool ErrorHistory::isConfirmed(const std::string& key, int repeatsRequired) const {
    return observationCount(key) >= repeatsRequired;
}

} // namespace pronunciation
