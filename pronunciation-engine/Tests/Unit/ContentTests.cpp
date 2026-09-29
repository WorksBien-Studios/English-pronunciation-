#include "Content/ContentStore.h"
#include "Content/PhonemeInventory.h"
#include "ContentValidation/ContentValidation.h"
#include "TestFramework.h"

#include <algorithm>
#include <cctype>
#include <fstream>

#include <nlohmann/json.hpp>

using namespace pronunciation;

namespace {
const std::string kResourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;
}

TEST(content_pack_loads_without_dangling_references) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);
    REQUIRE(!store.errorPatterns().empty());
    REQUIRE(!store.practiceWords().empty());
    REQUIRE(!store.lessons().empty());
}

TEST(content_pack_passes_structural_validation) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);
    ContentValidationReport report = ContentValidator::validate(store, inventory);
    for (const auto& issue : report.issues) {
        std::fprintf(stderr, "content issue at %s: %s\n", issue.location.c_str(), issue.message.c_str());
    }
    REQUIRE(report.isValid());
}

TEST(content_store_builds_exercise_for_word) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);

    ExerciseDefinition exercise = store.buildExerciseForWord("cat");
    REQUIRE(exercise.expectedPhonemes.size() == 3);
    REQUIRE(exercise.expectedPhonemes[0].phonemeId == inventory.idForSymbol("K"));
    REQUIRE(exercise.expectedPhonemes[1].phonemeId == inventory.idForSymbol("AE"));
    REQUIRE(exercise.expectedPhonemes[2].phonemeId == inventory.idForSymbol("T"));
}

TEST(content_store_attaches_confusion_set_for_targeted_substitution_pattern) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);

    ExerciseDefinition exercise = store.buildExerciseForWord("right");
    REQUIRE(exercise.expectedPhonemes[0].phonemeId == inventory.idForSymbol("R"));
    REQUIRE(exercise.expectedPhonemes[0].errorPatternId == "rl-substitution");
    REQUIRE(exercise.expectedPhonemes[0].confusablePhonemeIds.size() == 1);
    REQUIRE(exercise.expectedPhonemes[0].confusablePhonemeIds[0] == inventory.idForSymbol("L"));
}

TEST(content_store_builds_exercise_for_sentence_by_concatenating_words) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);

    ExerciseDefinition sentence = store.buildExerciseForSentence("the-right-light-is-red");
    size_t expectedLength = store.findWord("the")->phonemes.size() + store.findWord("right")->phonemes.size() +
                             store.findWord("light")->phonemes.size() + store.findWord("is")->phonemes.size() +
                             store.findWord("red")->phonemes.size();
    REQUIRE(sentence.expectedPhonemes.size() == expectedLength);
}

TEST(every_ios_stage_word_has_a_scoring_definition) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    ContentStore store = ContentStore::loadFromDirectory(kResourcesDir, inventory);
    std::ifstream input(PRONUNCIATION_ENGINE_IOS_STAGES_PATH);
    REQUIRE(static_cast<bool>(input));
    nlohmann::json stages;
    input >> stages;

    for (const auto& stage : stages.at("stages")) {
        for (const auto& word : stage.at("words")) {
            std::string id = word.at("text").get<std::string>();
            std::transform(id.begin(), id.end(), id.begin(),
                           [](unsigned char character) {
                               return static_cast<char>(std::tolower(character));
                           });
            REQUIRE(store.findWord(id) != nullptr);
        }
    }
}
