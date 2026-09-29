#include "ContentStore.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace pronunciation {

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace {

json loadJson(const fs::path& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("ContentStore: could not open " + path.string());
    }
    json doc;
    in >> doc;
    return doc;
}

ErrorPatternType parseErrorPatternType(const std::string& s) {
    if (s == "substitution") return ErrorPatternType::Substitution;
    if (s == "insertion") return ErrorPatternType::Insertion;
    if (s == "deletion") return ErrorPatternType::Deletion;
    throw std::runtime_error("ContentStore: unknown error pattern type " + s);
}

DiagnosticPromptType parsePromptType(const std::string& s) {
    if (s == "word") return DiagnosticPromptType::Word;
    if (s == "minimalPair") return DiagnosticPromptType::MinimalPair;
    if (s == "sentence") return DiagnosticPromptType::Sentence;
    throw std::runtime_error("ContentStore: unknown diagnostic prompt type " + s);
}

std::vector<std::string> stringArray(const json& node) {
    std::vector<std::string> out;
    for (const auto& v : node) out.push_back(v.get<std::string>());
    return out;
}

} // namespace

ContentStore ContentStore::loadFromDirectory(const std::string& resourcesDir, const PhonemeInventory& inventory) {
    ContentStore store;
    store.inventory_ = inventory;
    fs::path root(resourcesDir);

    // --- japanese-error-patterns.json ---
    json patternsDoc = loadJson(root / "japanese-error-patterns.json");
    for (const auto& entry : patternsDoc.at("errorPatterns")) {
        JapaneseErrorPattern pattern;
        pattern.id = entry.at("id").get<std::string>();
        pattern.type = parseErrorPatternType(entry.at("type").get<std::string>());
        if (!entry.at("expectedPhoneme").is_null()) {
            pattern.expectedPhonemeSymbol = entry.at("expectedPhoneme").get<std::string>();
        }
        pattern.confusablePhonemeSymbols = stringArray(entry.at("confusablePhonemes"));
        pattern.confidenceThreshold = entry.at("confidenceThreshold").get<float>();
        pattern.repeatsRequiredToConfirm = entry.at("repeatsRequiredToConfirm").get<int>();
        const auto& guide = entry.at("correctionGuide");
        pattern.correctionGuide.whatUserLikelyDidJa = guide.at("whatUserLikelyDidJa").get<std::string>();
        pattern.correctionGuide.whyJapaneseSpeakersJa = guide.at("whyJapaneseSpeakersJa").get<std::string>();
        pattern.correctionGuide.articulatoryCorrectionJa = guide.at("articulatoryCorrectionJa").get<std::string>();
        pattern.correctionGuide.analogyJa = guide.value("analogyJa", std::string());
        if (guide.contains("contrastSound") && !guide.at("contrastSound").is_null()) {
            pattern.correctionGuide.contrastSoundSymbol = guide.at("contrastSound").get<std::string>();
        }
        pattern.minimalPairIds = stringArray(entry.at("minimalPairIds"));

        if (!pattern.expectedPhonemeSymbol.empty() && !inventory.isValidSymbol(pattern.expectedPhonemeSymbol)) {
            throw std::runtime_error("ContentStore: error pattern " + pattern.id + " has unknown expectedPhoneme " + pattern.expectedPhonemeSymbol);
        }
        for (const auto& s : pattern.confusablePhonemeSymbols) {
            if (!inventory.isValidSymbol(s)) {
                throw std::runtime_error("ContentStore: error pattern " + pattern.id + " has unknown confusable phoneme " + s);
            }
        }
        store.errorPatterns_[pattern.id] = std::move(pattern);
    }

    // --- minimal-pairs.json ---
    json pairsDoc = loadJson(root / "minimal-pairs.json");
    for (const auto& entry : pairsDoc.at("minimalPairs")) {
        MinimalPair pair;
        pair.id = entry.at("id").get<std::string>();
        pair.errorPatternId = entry.at("errorPatternId").get<std::string>();
        pair.wordA = entry.at("wordA").get<std::string>();
        pair.phonemesA = stringArray(entry.at("phonemesA"));
        pair.wordB = entry.at("wordB").get<std::string>();
        pair.phonemesB = stringArray(entry.at("phonemesB"));
        pair.contrastPosition = entry.at("contrastPosition").get<int>();

        if (!store.errorPatterns_.count(pair.errorPatternId)) {
            throw std::runtime_error("ContentStore: minimal pair " + pair.id + " references unknown error pattern " + pair.errorPatternId);
        }
        store.minimalPairs_[pair.id] = std::move(pair);
    }

    // --- practice-words.json ---
    json wordsDoc = loadJson(root / "practice-words.json");
    for (const auto& entry : wordsDoc.at("practiceWords")) {
        PracticeWord word;
        word.id = entry.at("id").get<std::string>();
        word.text = entry.at("text").get<std::string>();
        word.ipa = entry.at("ipa").get<std::string>();
        word.phonemes = stringArray(entry.at("phonemes"));
        for (const auto& syl : entry.at("syllables")) {
            Syllable syllable;
            syllable.phonemes = stringArray(syl.at("phonemes"));
            syllable.stressed = syl.at("stressed").get<bool>();
            word.syllables.push_back(std::move(syllable));
        }
        word.targetErrorPatternIds = stringArray(entry.at("targetErrorPatternIds"));

        for (const auto& s : word.phonemes) {
            if (!inventory.isValidSymbol(s)) {
                throw std::runtime_error("ContentStore: word " + word.id + " has unknown phoneme " + s);
            }
        }
        for (const auto& patternId : word.targetErrorPatternIds) {
            if (!store.errorPatterns_.count(patternId)) {
                throw std::runtime_error("ContentStore: word " + word.id + " references unknown error pattern " + patternId);
            }
        }
        store.practiceWords_[word.id] = std::move(word);
    }

    // --- practice-sentences.json ---
    json sentencesDoc = loadJson(root / "practice-sentences.json");
    for (const auto& entry : sentencesDoc.at("practiceSentences")) {
        PracticeSentence sentence;
        sentence.id = entry.at("id").get<std::string>();
        sentence.text = entry.at("text").get<std::string>();
        sentence.wordIds = stringArray(entry.at("wordIds"));
        sentence.targetErrorPatternIds = stringArray(entry.at("targetErrorPatternIds"));

        for (const auto& wordId : sentence.wordIds) {
            if (!store.practiceWords_.count(wordId)) {
                throw std::runtime_error("ContentStore: sentence " + sentence.id + " references unknown word " + wordId);
            }
        }
        for (const auto& patternId : sentence.targetErrorPatternIds) {
            if (!store.errorPatterns_.count(patternId)) {
                throw std::runtime_error("ContentStore: sentence " + sentence.id + " references unknown error pattern " + patternId);
            }
        }
        store.practiceSentences_[sentence.id] = std::move(sentence);
    }

    // --- diagnostic-prompts.json ---
    json promptsDoc = loadJson(root / "diagnostic-prompts.json");
    for (const auto& entry : promptsDoc.at("diagnosticPrompts")) {
        DiagnosticPrompt prompt;
        prompt.id = entry.at("id").get<std::string>();
        prompt.order = entry.at("order").get<int>();
        prompt.promptType = parsePromptType(entry.at("promptType").get<std::string>());
        prompt.referenceId = entry.at("referenceId").get<std::string>();
        prompt.targetErrorPatternIds = stringArray(entry.at("targetErrorPatternIds"));

        bool referenceResolves = false;
        switch (prompt.promptType) {
            case DiagnosticPromptType::Word:
                referenceResolves = store.practiceWords_.count(prompt.referenceId) > 0;
                break;
            case DiagnosticPromptType::MinimalPair:
                referenceResolves = store.minimalPairs_.count(prompt.referenceId) > 0;
                break;
            case DiagnosticPromptType::Sentence:
                referenceResolves = store.practiceSentences_.count(prompt.referenceId) > 0;
                break;
        }
        if (!referenceResolves) {
            throw std::runtime_error("ContentStore: diagnostic prompt " + prompt.id + " references unknown item " + prompt.referenceId);
        }
        store.diagnosticPrompts_.push_back(std::move(prompt));
    }

    // --- lessons/*.json ---
    fs::path lessonsDir = root / "lessons";
    if (fs::exists(lessonsDir)) {
        for (const auto& file : fs::directory_iterator(lessonsDir)) {
            if (file.path().extension() != ".json") continue;
            json lessonDoc = loadJson(file.path());
            Lesson lesson;
            lesson.id = lessonDoc.at("id").get<std::string>();
            lesson.errorPatternId = lessonDoc.at("errorPatternId").get<std::string>();
            lesson.title = lessonDoc.at("title").get<std::string>();
            lesson.order = lessonDoc.at("order").get<int>();
            lesson.minimalPairIds = stringArray(lessonDoc.at("minimalPairIds"));
            lesson.practiceWordIds = stringArray(lessonDoc.at("practiceWordIds"));
            lesson.practiceSentenceIds = stringArray(lessonDoc.at("practiceSentenceIds"));
            lesson.successThreshold = lessonDoc.at("successThreshold").get<float>();

            if (!store.errorPatterns_.count(lesson.errorPatternId)) {
                throw std::runtime_error("ContentStore: lesson " + lesson.id + " references unknown error pattern " + lesson.errorPatternId);
            }
            for (const auto& id : lesson.minimalPairIds) {
                if (!store.minimalPairs_.count(id)) throw std::runtime_error("ContentStore: lesson " + lesson.id + " references unknown minimal pair " + id);
            }
            for (const auto& id : lesson.practiceWordIds) {
                if (!store.practiceWords_.count(id)) throw std::runtime_error("ContentStore: lesson " + lesson.id + " references unknown word " + id);
            }
            for (const auto& id : lesson.practiceSentenceIds) {
                if (!store.practiceSentences_.count(id)) throw std::runtime_error("ContentStore: lesson " + lesson.id + " references unknown sentence " + id);
            }
            store.lessons_[lesson.id] = std::move(lesson);
        }
    }

    // --- assessment-rules.json ---
    json rulesDoc = loadJson(root / "assessment-rules.json");
    for (const auto& entry : rulesDoc.at("assessmentRules")) {
        AssessmentRule rule;
        rule.id = entry.at("id").get<std::string>();
        rule.appliesToErrorPatternId = entry.at("appliesToErrorPatternId").get<std::string>();
        rule.confidenceThreshold = entry.at("confidenceThreshold").get<float>();
        rule.minRepeatsToConfirm = entry.at("minRepeatsToConfirm").get<int>();
        rule.recommendedLessonId = entry.at("recommendedLessonId").get<std::string>();

        if (!store.errorPatterns_.count(rule.appliesToErrorPatternId)) {
            throw std::runtime_error("ContentStore: assessment rule " + rule.id + " references unknown error pattern " + rule.appliesToErrorPatternId);
        }
        if (!store.lessons_.count(rule.recommendedLessonId)) {
            throw std::runtime_error("ContentStore: assessment rule " + rule.id + " references unknown lesson " + rule.recommendedLessonId);
        }
        store.assessmentRules_[rule.id] = std::move(rule);
    }

    return store;
}

const JapaneseErrorPattern* ContentStore::findErrorPattern(const std::string& id) const {
    auto it = errorPatterns_.find(id);
    return it == errorPatterns_.end() ? nullptr : &it->second;
}

const PracticeWord* ContentStore::findWord(const std::string& id) const {
    auto it = practiceWords_.find(id);
    return it == practiceWords_.end() ? nullptr : &it->second;
}

const MinimalPair* ContentStore::findMinimalPair(const std::string& id) const {
    auto it = minimalPairs_.find(id);
    return it == minimalPairs_.end() ? nullptr : &it->second;
}

const PracticeSentence* ContentStore::findSentence(const std::string& id) const {
    auto it = practiceSentences_.find(id);
    return it == practiceSentences_.end() ? nullptr : &it->second;
}

const AssessmentRule* ContentStore::findRuleForErrorPattern(const std::string& errorPatternId) const {
    for (const auto& [id, rule] : assessmentRules_) {
        if (rule.appliesToErrorPatternId == errorPatternId) return &rule;
    }
    return nullptr;
}

ExerciseDefinition ContentStore::buildExerciseFromPhonemes(const std::vector<std::string>& phonemeSymbols,
                                                            const std::vector<std::string>& targetErrorPatternIds) const {
    ExerciseDefinition exercise;
    for (const auto& symbol : phonemeSymbols) {
        ExpectedPhonemeSlot slot;
        slot.phonemeId = inventory_.idForSymbol(symbol);
        auto info = inventory_.infoForId(slot.phonemeId);
        slot.isVowel = info && info->isVowel;

        // Attach the Japanese confusion set for the first targeted
        // substitution-type error pattern whose expected phoneme matches
        // this slot. Insertion/deletion-type patterns are exercise-level
        // (see DecisionRules), not tied to one slot.
        for (const auto& patternId : targetErrorPatternIds) {
            const JapaneseErrorPattern* pattern = findErrorPattern(patternId);
            if (pattern && pattern->type == ErrorPatternType::Substitution && pattern->expectedPhonemeSymbol == symbol) {
                slot.errorPatternId = pattern->id;
                slot.confusablePhonemeIds = inventory_.symbolsToIds(pattern->confusablePhonemeSymbols);
                break;
            }
        }
        exercise.expectedPhonemes.push_back(std::move(slot));
    }
    return exercise;
}

ExerciseDefinition ContentStore::buildExerciseForWord(const std::string& wordId) const {
    const PracticeWord* word = findWord(wordId);
    if (!word) throw std::runtime_error("ContentStore: unknown word " + wordId);
    ExerciseDefinition exercise = buildExerciseFromPhonemes(word->phonemes, word->targetErrorPatternIds);
    exercise.exerciseId = "word:" + wordId;
    return exercise;
}

ExerciseDefinition ContentStore::buildExerciseForMinimalPair(const std::string& pairId, bool isSideA) const {
    const MinimalPair* pair = findMinimalPair(pairId);
    if (!pair) throw std::runtime_error("ContentStore: unknown minimal pair " + pairId);
    const auto& phonemes = isSideA ? pair->phonemesA : pair->phonemesB;
    ExerciseDefinition exercise = buildExerciseFromPhonemes(phonemes, {pair->errorPatternId});
    exercise.exerciseId = "minimalPair:" + pairId + (isSideA ? ":A" : ":B");
    return exercise;
}

ExerciseDefinition ContentStore::buildExerciseForSentence(const std::string& sentenceId) const {
    const PracticeSentence* sentence = findSentence(sentenceId);
    if (!sentence) throw std::runtime_error("ContentStore: unknown sentence " + sentenceId);

    ExerciseDefinition exercise;
    exercise.exerciseId = "sentence:" + sentenceId;
    for (const auto& wordId : sentence->wordIds) {
        const PracticeWord* word = findWord(wordId);
        ExerciseDefinition wordExercise = buildExerciseFromPhonemes(word->phonemes, sentence->targetErrorPatternIds);
        for (auto& slot : wordExercise.expectedPhonemes) {
            exercise.expectedPhonemes.push_back(std::move(slot));
        }
    }
    return exercise;
}

} // namespace pronunciation
