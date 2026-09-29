#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "ContentTypes.h"
#include "PhonemeInventory.h"

namespace pronunciation {

// Loads and indexes the versioned JSON content pack described in
// docs/product-specification.md#content-format. Construction fails fast
// (throws std::runtime_error) on any dangling reference between content
// files; ContentValidator (ContentValidation.h) additionally provides a
// non-throwing report for content-authoring/lint tooling.
class ContentStore {
public:
    static ContentStore loadFromDirectory(const std::string& resourcesDir, const PhonemeInventory& inventory);

    const std::unordered_map<std::string, JapaneseErrorPattern>& errorPatterns() const { return errorPatterns_; }
    const std::unordered_map<std::string, MinimalPair>& minimalPairs() const { return minimalPairs_; }
    const std::unordered_map<std::string, PracticeWord>& practiceWords() const { return practiceWords_; }
    const std::unordered_map<std::string, PracticeSentence>& practiceSentences() const { return practiceSentences_; }
    const std::vector<DiagnosticPrompt>& diagnosticPrompts() const { return diagnosticPrompts_; }
    const std::unordered_map<std::string, Lesson>& lessons() const { return lessons_; }
    const std::unordered_map<std::string, AssessmentRule>& assessmentRules() const { return assessmentRules_; }

    const JapaneseErrorPattern* findErrorPattern(const std::string& id) const;
    const PracticeWord* findWord(const std::string& id) const;
    const MinimalPair* findMinimalPair(const std::string& id) const;
    const PracticeSentence* findSentence(const std::string& id) const;
    const AssessmentRule* findRuleForErrorPattern(const std::string& errorPatternId) const;

    // Resolves a practice word into an engine-ready exercise: its phoneme
    // sequence, with the Japanese confusion set attached to any slot whose
    // symbol matches a substitution-type error pattern the word targets.
    ExerciseDefinition buildExerciseForWord(const std::string& wordId) const;

    // Resolves one side of a minimal pair (isSideA selects word A vs B).
    ExerciseDefinition buildExerciseForMinimalPair(const std::string& pairId, bool isSideA) const;

    // Concatenates each referenced word's phoneme sequence in order.
    ExerciseDefinition buildExerciseForSentence(const std::string& sentenceId) const;

    const PhonemeInventory& inventory() const { return inventory_; }

private:
    PhonemeInventory inventory_;
    std::unordered_map<std::string, JapaneseErrorPattern> errorPatterns_;
    std::unordered_map<std::string, MinimalPair> minimalPairs_;
    std::unordered_map<std::string, PracticeWord> practiceWords_;
    std::unordered_map<std::string, PracticeSentence> practiceSentences_;
    std::vector<DiagnosticPrompt> diagnosticPrompts_;
    std::unordered_map<std::string, Lesson> lessons_;
    std::unordered_map<std::string, AssessmentRule> assessmentRules_;

    ExerciseDefinition buildExerciseFromPhonemes(const std::vector<std::string>& phonemeSymbols,
                                                  const std::vector<std::string>& targetErrorPatternIds) const;
};

} // namespace pronunciation
