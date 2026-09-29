#include "PhonemeInventory.h"

#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace pronunciation {

using json = nlohmann::json;

PhonemeInventory PhonemeInventory::loadFromFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("PhonemeInventory: could not open " + path);
    }
    json doc;
    in >> doc;

    PhonemeInventory inventory;
    PhonemeId nextId = 0;
    for (const auto& entry : doc.at("phonemes")) {
        PhonemeInfo info;
        info.id = nextId++;
        info.symbol = entry.at("symbol").get<std::string>();
        info.ipa = entry.at("ipa").get<std::string>();
        info.isVowel = entry.at("type").get<std::string>() == "vowel";
        info.isSchwa = entry.value("isSchwa", false);

        if (inventory.symbolToId_.count(info.symbol)) {
            throw std::runtime_error("PhonemeInventory: duplicate symbol " + info.symbol);
        }
        inventory.symbolToId_[info.symbol] = info.id;
        inventory.infos_.push_back(std::move(info));
    }
    return inventory;
}

PhonemeId PhonemeInventory::idForSymbol(const std::string& symbol) const {
    auto it = symbolToId_.find(symbol);
    return it == symbolToId_.end() ? kInvalidPhonemeId : it->second;
}

std::optional<PhonemeInfo> PhonemeInventory::infoForId(PhonemeId id) const {
    if (id < 0 || static_cast<size_t>(id) >= infos_.size()) {
        return std::nullopt;
    }
    return infos_[static_cast<size_t>(id)];
}

bool PhonemeInventory::isValidSymbol(const std::string& symbol) const {
    return symbolToId_.count(symbol) > 0;
}

std::vector<PhonemeId> PhonemeInventory::symbolsToIds(const std::vector<std::string>& symbols) const {
    std::vector<PhonemeId> ids;
    ids.reserve(symbols.size());
    for (const auto& symbol : symbols) {
        PhonemeId id = idForSymbol(symbol);
        if (id == kInvalidPhonemeId) {
            throw std::runtime_error("PhonemeInventory: unknown phoneme symbol " + symbol);
        }
        ids.push_back(id);
    }
    return ids;
}

} // namespace pronunciation
