#include "ModelOutputProjection.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <unordered_map>

#include <nlohmann/json.hpp>

namespace pronunciation {

namespace {

using json = nlohmann::json;
constexpr float kNegativeInfinity = -std::numeric_limits<float>::infinity();

json loadJson(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("ModelOutputProjection: could not open " + path);
    json document;
    input >> document;
    return document;
}

float logAdd(float left, float right) {
    if (left == kNegativeInfinity) return right;
    if (right == kNegativeInfinity) return left;
    const float high = std::max(left, right);
    const float low = std::min(left, right);
    return high + std::log1p(std::exp(low - high));
}

} // namespace

ModelOutputProjection ModelOutputProjection::loadFromFiles(
    const std::string& phonemeInventoryPath,
    const std::string& mappingPath) {
    const json inventoryDocument = loadJson(phonemeInventoryPath);
    const json mappingDocument = loadJson(mappingPath);

    std::unordered_map<std::string, int> internalColumns;
    const auto& phonemes = inventoryDocument.at("phonemes");
    for (size_t index = 0; index < phonemes.size(); ++index) {
        const std::string symbol = phonemes.at(index).at("symbol").get<std::string>();
        if (!internalColumns.emplace(symbol, static_cast<int>(index)).second) {
            throw std::runtime_error("ModelOutputProjection: duplicate internal symbol " + symbol);
        }
    }
    if (internalColumns.empty()) {
        throw std::runtime_error("ModelOutputProjection: empty internal inventory");
    }

    ModelOutputProjection projection;
    projection.internalVocabularySize_ = static_cast<int>(internalColumns.size());
    projection.modelVocabularySize_ = mappingDocument.at("modelVocabularySize").get<int>();
    projection.modelBlankTokenId_ = mappingDocument.at("modelBlankTokenId").get<int>();
    if (projection.modelVocabularySize_ <= 0 ||
        projection.modelBlankTokenId_ < 0 ||
        projection.modelBlankTokenId_ >= projection.modelVocabularySize_) {
        throw std::runtime_error("ModelOutputProjection: invalid model vocabulary metadata");
    }
    projection.mappingsByModelToken_.resize(
        static_cast<size_t>(projection.modelVocabularySize_));

    std::vector<bool> internalSymbolCovered(internalColumns.size(), false);
    std::vector<bool> modelTokenSeen(
        static_cast<size_t>(projection.modelVocabularySize_), false);

    for (const auto& entry : mappingDocument.at("tokenMappings")) {
        const int modelTokenId = entry.at("modelTokenId").get<int>();
        if (modelTokenId < 0 || modelTokenId >= projection.modelVocabularySize_ ||
            modelTokenId == projection.modelBlankTokenId_) {
            throw std::runtime_error("ModelOutputProjection: invalid mapped model token");
        }
        if (modelTokenSeen.at(static_cast<size_t>(modelTokenId))) {
            throw std::runtime_error("ModelOutputProjection: duplicate mapped model token " +
                                     std::to_string(modelTokenId));
        }
        modelTokenSeen[static_cast<size_t>(modelTokenId)] = true;

        auto& mappedColumns =
            projection.mappingsByModelToken_[static_cast<size_t>(modelTokenId)]
                .internalColumns;
        for (const auto& symbolNode : entry.at("internalSymbols")) {
            const std::string symbol = symbolNode.get<std::string>();
            const auto found = internalColumns.find(symbol);
            if (found == internalColumns.end()) {
                throw std::runtime_error(
                    "ModelOutputProjection: unknown internal symbol " + symbol);
            }
            if (std::find(mappedColumns.begin(), mappedColumns.end(), found->second) !=
                mappedColumns.end()) {
                throw std::runtime_error(
                    "ModelOutputProjection: duplicate internal symbol in token mapping " +
                    symbol);
            }
            mappedColumns.push_back(found->second);
            internalSymbolCovered[static_cast<size_t>(found->second)] = true;
        }
        if (mappedColumns.empty()) {
            throw std::runtime_error("ModelOutputProjection: empty token mapping");
        }
    }

    for (const auto& [symbol, column] : internalColumns) {
        if (!internalSymbolCovered.at(static_cast<size_t>(column))) {
            throw std::runtime_error(
                "ModelOutputProjection: no model token maps to " + symbol);
        }
    }
    return projection;
}

FrameLogProbs ModelOutputProjection::project(
    const float* rawLogits,
    int numFrames,
    int modelVocabularySize,
    double frameDurationSeconds) const {
    if (!rawLogits || numFrames <= 0 ||
        modelVocabularySize != modelVocabularySize_ ||
        !(frameDurationSeconds > 0.0) || !std::isfinite(frameDurationSeconds)) {
        throw std::invalid_argument("ModelOutputProjection: invalid inference output");
    }

    FrameLogProbs projected;
    projected.numFrames = numFrames;
    projected.vocabSize = vocabularySize();
    projected.blankColumn = blankColumn();
    projected.frameDurationSeconds = frameDurationSeconds;
    projected.data.assign(
        static_cast<size_t>(numFrames) * static_cast<size_t>(projected.vocabSize),
        kNegativeInfinity);

    for (int frame = 0; frame < numFrames; ++frame) {
        const float* row = rawLogits +
                           static_cast<size_t>(frame) *
                               static_cast<size_t>(modelVocabularySize_);
        float maxLogit = kNegativeInfinity;
        for (int token = 0; token < modelVocabularySize_; ++token) {
            if (!std::isfinite(row[token])) {
                throw std::runtime_error(
                    "ModelOutputProjection: model emitted non-finite logits");
            }
            maxLogit = std::max(maxLogit, row[token]);
        }

        double denominator = 0.0;
        for (int token = 0; token < modelVocabularySize_; ++token) {
            denominator += std::exp(static_cast<double>(row[token] - maxLogit));
        }
        const float logDenominator =
            maxLogit + static_cast<float>(std::log(denominator));
        float* output = projected.data.data() +
                        static_cast<size_t>(frame) *
                            static_cast<size_t>(projected.vocabSize);

        for (int token = 0; token < modelVocabularySize_; ++token) {
            const float logProbability = row[token] - logDenominator;
            if (token == modelBlankTokenId_) {
                output[blankColumn()] = logAdd(output[blankColumn()], logProbability);
                continue;
            }

            const auto& mapped =
                mappingsByModelToken_[static_cast<size_t>(token)].internalColumns;
            if (mapped.empty()) {
                output[unknownColumn()] =
                    logAdd(output[unknownColumn()], logProbability);
                continue;
            }

            const float distributed =
                logProbability - std::log(static_cast<float>(mapped.size()));
            for (int internalColumn : mapped) {
                output[internalColumn] =
                    logAdd(output[internalColumn], distributed);
            }
        }
    }
    return projected;
}

} // namespace pronunciation
