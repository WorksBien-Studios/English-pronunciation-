#pragma once

#include <string>
#include <vector>

#include "../Content/ContentStore.h"
#include "../Content/PhonemeInventory.h"

namespace pronunciation {

struct ContentValidationIssue {
    std::string location; // e.g. "minimalPairs/right-light"
    std::string message;
};

struct ContentValidationReport {
    std::vector<ContentValidationIssue> issues;
    bool isValid() const { return issues.empty(); }
};

// ContentStore::loadFromDirectory already fails fast on dangling references.
// This validator runs additional structural checks that a launch content
// review needs but that do not by themselves make the content unusable
// (e.g. a minimal pair whose two sides do not actually differ in exactly
// one phoneme), so they are reported rather than thrown.
class ContentValidator {
public:
    static ContentValidationReport validate(const ContentStore& store, const PhonemeInventory& inventory);
};

} // namespace pronunciation
