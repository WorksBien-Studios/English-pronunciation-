#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../Types.h"

namespace pronunciation {

struct PhonemeInfo {
    PhonemeId id = kInvalidPhonemeId;
    std::string symbol;
    std::string ipa;
    bool isVowel = false;
    bool isSchwa = false;
};

// Loads Resources/phonemes.json and assigns each ARPAbet-like symbol a
// stable integer id used throughout the engine's hot path. The CTC blank
// unit is not part of this inventory; acoustic-model backends append it as
// vocabulary index == phonemes.size() (see AcousticModel::vocabularySize()).
class PhonemeInventory {
public:
    static PhonemeInventory loadFromFile(const std::string& path);

    PhonemeId idForSymbol(const std::string& symbol) const;
    std::optional<PhonemeInfo> infoForId(PhonemeId id) const;
    size_t size() const { return infos_.size(); }
    bool isValidSymbol(const std::string& symbol) const;

    std::vector<PhonemeId> symbolsToIds(const std::vector<std::string>& symbols) const;

private:
    std::vector<PhonemeInfo> infos_; // indexed by PhonemeId
    std::unordered_map<std::string, PhonemeId> symbolToId_;
};

} // namespace pronunciation
