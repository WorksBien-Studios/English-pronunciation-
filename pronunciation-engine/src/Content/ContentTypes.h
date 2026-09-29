#pragma once

#include <string>
#include <vector>

#include "../Types.h"

namespace pronunciation {

struct CorrectionGuide {
    std::string whatUserLikelyDidJa;
    std::string whyJapaneseSpeakersJa;
    std::string articulatoryCorrectionJa;
    std::string analogyJa;
    std::string contrastSoundSymbol; // may be empty
};

enum class ErrorPatternType {
    Substitution,
    Insertion,
    Deletion,
};

struct JapaneseErrorPattern {
    std::string id;
    ErrorPatternType type = ErrorPatternType::Substitution;
    std::string expectedPhonemeSymbol; // empty for Insertion/Deletion patterns
    std::vector<std::string> confusablePhonemeSymbols;
    float confidenceThreshold = 0.6f;
    int repeatsRequiredToConfirm = 2;
    CorrectionGuide correctionGuide;
    std::vector<std::string> minimalPairIds;
};

struct MinimalPair {
    std::string id;
    std::string errorPatternId;
    std::string wordA;
    std::vector<std::string> phonemesA;
    std::string wordB;
    std::vector<std::string> phonemesB;
    int contrastPosition = 0;
};

struct Syllable {
    std::vector<std::string> phonemes;
    bool stressed = false;
};

struct PracticeWord {
    std::string id;
    std::string text;
    std::string ipa;
    std::vector<std::string> phonemes;
    std::vector<Syllable> syllables;
    std::vector<std::string> targetErrorPatternIds;
};

struct PracticeSentence {
    std::string id;
    std::string text;
    std::vector<std::string> wordIds;
    std::vector<std::string> targetErrorPatternIds;
};

enum class DiagnosticPromptType {
    Word,
    MinimalPair,
    Sentence,
};

struct DiagnosticPrompt {
    std::string id;
    int order = 0;
    DiagnosticPromptType promptType = DiagnosticPromptType::Word;
    std::string referenceId;
    std::vector<std::string> targetErrorPatternIds;
};

struct Lesson {
    std::string id;
    std::string errorPatternId;
    std::string title;
    int order = 0;
    std::vector<std::string> minimalPairIds;
    std::vector<std::string> practiceWordIds;
    std::vector<std::string> practiceSentenceIds;
    float successThreshold = 0.6f;
};

struct AssessmentRule {
    std::string id;
    std::string appliesToErrorPatternId;
    float confidenceThreshold = 0.6f;
    int minRepeatsToConfirm = 2;
    std::string recommendedLessonId;
};

} // namespace pronunciation
