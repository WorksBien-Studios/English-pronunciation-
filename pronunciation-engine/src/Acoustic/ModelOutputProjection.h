#pragma once

#include <string>
#include <vector>

#include "../Types.h"

namespace pronunciation {

// Converts the locked model's 392 eSpeak/IPA CTC logits into the engine's
// compact phoneme inventory while preserving both explicit CTC blank mass and
// all unmapped evidence in a separate conservative "unknown" column.
class ModelOutputProjection {
public:
    static ModelOutputProjection loadFromFiles(
        const std::string& phonemeInventoryPath,
        const std::string& mappingPath);

    FrameLogProbs project(
        const float* rawLogits,
        int numFrames,
        int modelVocabularySize,
        double frameDurationSeconds) const;

    int vocabularySize() const { return internalVocabularySize_ + 2; }
    int unknownColumn() const { return internalVocabularySize_; }
    int blankColumn() const { return internalVocabularySize_ + 1; }
    int modelVocabularySize() const { return modelVocabularySize_; }

private:
    struct TokenMapping {
        std::vector<int> internalColumns;
    };

    int internalVocabularySize_ = 0;
    int modelVocabularySize_ = 0;
    int modelBlankTokenId_ = -1;
    std::vector<TokenMapping> mappingsByModelToken_;
};

} // namespace pronunciation
