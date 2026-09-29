#include "ContentValidation.h"

namespace pronunciation {

namespace {

void checkRange01(ContentValidationReport& report, const std::string& location, const std::string& field, float value) {
    if (value < 0.0f || value > 1.0f) {
        report.issues.push_back({location, field + " must be in [0, 1], got " + std::to_string(value)});
    }
}

void validateMinimalPair(ContentValidationReport& report, const MinimalPair& pair) {
    const std::string location = "minimalPairs/" + pair.id;
    if (pair.phonemesA.size() != pair.phonemesB.size()) {
        report.issues.push_back({location, "wordA/wordB phoneme sequences must be equal length for a minimal pair"});
        return;
    }
    int differingPositions = 0;
    int firstDiffPosition = -1;
    for (size_t i = 0; i < pair.phonemesA.size(); ++i) {
        if (pair.phonemesA[i] != pair.phonemesB[i]) {
            ++differingPositions;
            if (firstDiffPosition < 0) firstDiffPosition = static_cast<int>(i);
        }
    }
    if (differingPositions != 1) {
        report.issues.push_back({location, "a minimal pair must differ in exactly one phoneme position, found " + std::to_string(differingPositions)});
    } else if (firstDiffPosition != pair.contrastPosition) {
        report.issues.push_back({location, "contrastPosition does not match the actual differing index"});
    }
}

void validateWord(ContentValidationReport& report, const PracticeWord& word) {
    const std::string location = "practiceWords/" + word.id;
    std::vector<std::string> fromSyllables;
    for (const auto& syllable : word.syllables) {
        for (const auto& p : syllable.phonemes) fromSyllables.push_back(p);
    }
    if (fromSyllables != word.phonemes) {
        report.issues.push_back({location, "concatenated syllable phonemes do not match the word's phoneme sequence"});
    }
    bool anyStressed = false;
    for (const auto& syllable : word.syllables) anyStressed = anyStressed || syllable.stressed;
    if (word.syllables.size() > 1 && !anyStressed) {
        report.issues.push_back({location, "multi-syllable word content should mark at least one stressed syllable"});
    }
}

void validateSentence(ContentValidationReport& report, const PracticeSentence& sentence) {
    const std::string location = "practiceSentences/" + sentence.id;
    if (sentence.wordIds.empty()) {
        report.issues.push_back({location, "a practice sentence must reference at least one word"});
    }
}

void validateErrorPattern(ContentValidationReport& report, const JapaneseErrorPattern& pattern) {
    const std::string location = "errorPatterns/" + pattern.id;
    checkRange01(report, location, "confidenceThreshold", pattern.confidenceThreshold);
    if (pattern.repeatsRequiredToConfirm < 1) {
        report.issues.push_back({location, "repeatsRequiredToConfirm must be >= 1"});
    }
    if (pattern.type == ErrorPatternType::Substitution) {
        if (pattern.expectedPhonemeSymbol.empty()) {
            report.issues.push_back({location, "substitution-type patterns must set expectedPhoneme"});
        }
        if (pattern.confusablePhonemeSymbols.empty()) {
            report.issues.push_back({location, "substitution-type patterns must list at least one confusable phoneme"});
        }
    }
    if (pattern.correctionGuide.whatUserLikelyDidJa.empty() || pattern.correctionGuide.whyJapaneseSpeakersJa.empty() ||
        pattern.correctionGuide.articulatoryCorrectionJa.empty()) {
        report.issues.push_back({location, "correctionGuide is missing required Japanese instructional fields"});
    }
}

void validateLesson(ContentValidationReport& report, const Lesson& lesson) {
    const std::string location = "lessons/" + lesson.id;
    checkRange01(report, location, "successThreshold", lesson.successThreshold);
    if (lesson.minimalPairIds.empty() && lesson.practiceWordIds.empty() && lesson.practiceSentenceIds.empty()) {
        report.issues.push_back({location, "a lesson must include at least one drill (minimal pair, word, or sentence)"});
    }
}

} // namespace

ContentValidationReport ContentValidator::validate(const ContentStore& store, const PhonemeInventory&) {
    ContentValidationReport report;
    for (const auto& [id, pair] : store.minimalPairs()) validateMinimalPair(report, pair);
    for (const auto& [id, word] : store.practiceWords()) validateWord(report, word);
    for (const auto& [id, sentence] : store.practiceSentences()) validateSentence(report, sentence);
    for (const auto& [id, pattern] : store.errorPatterns()) validateErrorPattern(report, pattern);
    for (const auto& [id, lesson] : store.lessons()) validateLesson(report, lesson);
    return report;
}

} // namespace pronunciation
