#include "Acoustic/ModelOutputProjection.h"
#include "Content/PhonemeInventory.h"
#include "TestFramework.h"

#include <cmath>
#include <limits>
#include <vector>

using namespace pronunciation;

namespace {

const std::string kResourcesDir = PRONUNCIATION_ENGINE_RESOURCES_DIR;

ModelOutputProjection loadProjection() {
    return ModelOutputProjection::loadFromFiles(
        kResourcesDir + "/phonemes.json",
        kResourcesDir + "/model-output-map.json");
}

std::vector<float> logitsWithWinner(int winner) {
    std::vector<float> logits(392, -12.0f);
    logits.at(static_cast<size_t>(winner)) = 12.0f;
    return logits;
}

float probabilitySum(const FrameLogProbs& projected) {
    float total = 0.0f;
    for (float logProbability : projected.data) total += std::exp(logProbability);
    return total;
}

} // namespace

TEST(model_projection_maps_single_ipa_token_to_internal_phoneme) {
    auto projection = loadProjection();
    auto inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    std::vector<float> logits = logitsWithWinner(27); // model token /ɹ/
    FrameLogProbs result = projection.project(logits.data(), 1, 392, 0.02);

    REQUIRE(result.vocabSize == static_cast<int>(inventory.size()) + 2);
    REQUIRE(result.blankColumn == static_cast<int>(inventory.size()) + 1);
    REQUIRE(result.at(0, inventory.idForSymbol("R")) > -0.001f);
    REQUIRE(std::fabs(probabilitySum(result) - 1.0f) < 0.0001f);
}

TEST(model_projection_splits_composite_token_without_losing_probability_mass) {
    auto projection = loadProjection();
    auto inventory = PhonemeInventory::loadFromFile(kResourcesDir + "/phonemes.json");
    std::vector<float> logits = logitsWithWinner(131); // model token /aɪɚ/
    FrameLogProbs result = projection.project(logits.data(), 1, 392, 0.02);

    const float ay = std::exp(result.at(0, inventory.idForSymbol("AY")));
    const float er = std::exp(result.at(0, inventory.idForSymbol("ER")));
    REQUIRE(std::fabs(ay - 0.5f) < 0.001f);
    REQUIRE(std::fabs(er - 0.5f) < 0.001f);
    REQUIRE(std::fabs(probabilitySum(result) - 1.0f) < 0.0001f);
}

TEST(model_projection_preserves_blank_and_unmapped_evidence) {
    auto projection = loadProjection();

    std::vector<float> blankLogits = logitsWithWinner(0);
    FrameLogProbs blank = projection.project(blankLogits.data(), 1, 392, 0.02);
    REQUIRE(blank.at(0, blank.blankColumn) > -0.001f);

    std::vector<float> unknownLogits = logitsWithWinner(313); // /ɸ/, not in engine inventory
    FrameLogProbs unknown = projection.project(unknownLogits.data(), 1, 392, 0.02);
    REQUIRE(unknown.at(0, projection.unknownColumn()) > -0.001f);
}

TEST(model_projection_rejects_non_finite_or_wrong_shape_output) {
    auto projection = loadProjection();
    std::vector<float> logits = logitsWithWinner(27);
    logits[5] = std::numeric_limits<float>::quiet_NaN();
    bool threw = false;
    try {
        (void)projection.project(logits.data(), 1, 392, 0.02);
    } catch (const std::exception&) {
        threw = true;
    }
    REQUIRE(threw);

    threw = false;
    try {
        (void)projection.project(logits.data(), 1, 391, 0.02);
    } catch (const std::exception&) {
        threw = true;
    }
    REQUIRE(threw);
}
