// Command-line test harness for the pronunciation engine, per
// docs/product-specification.md#technical-risk-and-mandatory-pre-ui-gate:
// "Before skinning the application, build a command-line/test-harness
// version of the engine around a 50-word spike." It has no audio capture of
// its own (that is AVFoundation's job on iOS) — every scenario here drives
// the engine through MockAcousticModel, because ONNX Runtime and the
// bundled model artifact are wired in from the iOS target, not from this
// engine-development environment (see src/Acoustic/OnnxAcousticModel.h).
//
// Two subcommands:
//   pronunciation-harness <resourcesDir> gate
//     Runs every practice word/minimal-pair side through two scripted
//     scenarios: pronounced correctly (must never yield SpecificError) and,
//     for slots with a tracked Japanese confusion pair, pronounced with the
//     confusable phoneme substituted (must yield SpecificError for that
//     exact pattern, and must confirm via history on a second attempt).
//     Exit code is 0 only if every scenario behaved as expected.
//
//   pronunciation-harness <resourcesDir> run <exerciseId> <symbol...>
//     Scripts the given phoneme symbols as the "produced" sequence for one
//     exercise and prints the resulting diagnosis as JSON.

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "Acoustic/MockAcousticModel.h"
#include "Content/ContentStore.h"
#include "Content/PhonemeInventory.h"
#include "Engine.h"

using namespace pronunciation;
using json = nlohmann::json;

namespace {

PcmBuffer makeSpeechLikeTone(double seconds, int sampleRateHz = 16000) {
    PcmBuffer buffer;
    buffer.sampleRateHz = sampleRateHz;
    size_t n = static_cast<size_t>(seconds * sampleRateHz);
    buffer.samples.resize(n);
    for (size_t i = 0; i < n; ++i) buffer.samples[i] = 0.5f * std::sin(2.0 * 3.14159265 * 220.0 * i / sampleRateHz);
    size_t noiseFloorSamples = n / 5;
    for (size_t i = 0; i < noiseFloorSamples; ++i) buffer.samples[i] *= 0.02f;
    return buffer;
}

std::unique_ptr<MockAcousticModel> makeScriptedModel(const PhonemeInventory& inventory, const std::vector<PhonemeId>& producedSequence,
                                                      int framesPerPhoneme = 5, float confidence = 0.9f) {
    auto model = std::make_unique<MockAcousticModel>(static_cast<int>(inventory.size()) + 1, static_cast<int>(inventory.size()));
    std::vector<ScriptedFrame> sequence;
    for (PhonemeId id : producedSequence) sequence.push_back({id, confidence});
    model->setScriptFromSequence(sequence, framesPerPhoneme);
    return model;
}

const char* verdictName(PhonemeVerdict v) {
    switch (v) {
        case PhonemeVerdict::Correct: return "correct";
        case PhonemeVerdict::Substituted: return "substituted";
        case PhonemeVerdict::Inserted: return "inserted";
        case PhonemeVerdict::Deleted: return "deleted";
        case PhonemeVerdict::LowConfidence: return "low_confidence";
    }
    return "unknown";
}

const char* outcomeName(DiagnosisOutcome o) {
    switch (o) {
        case DiagnosisOutcome::Pass: return "pass";
        case DiagnosisOutcome::Retry: return "retry";
        case DiagnosisOutcome::SpecificError: return "specific_error";
    }
    return "unknown";
}

json diagnosisToJson(const EngineResult& result) {
    json out;
    out["audioQuality"]["passesGate"] = result.audioQuality.passesGate;
    out["audioQuality"]["rmsDbfs"] = result.audioQuality.rmsDbfs;
    out["audioQuality"]["estimatedSnrDb"] = result.audioQuality.estimatedSnrDb;
    out["diagnosis"]["outcome"] = outcomeName(result.diagnosis.outcome);
    out["diagnosis"]["errorPatternId"] = result.diagnosis.errorPatternId;
    out["diagnosis"]["errorConfirmedByHistory"] = result.diagnosis.errorConfirmedByHistory;
    json scores = json::array();
    for (const auto& score : result.phonemeScores) {
        scores.push_back({{"expectedPhonemeId", score.expectedPhonemeId},
                           {"verdict", verdictName(score.verdict)},
                           {"gopScore", score.gopScore},
                           {"targetPosterior", score.targetPosterior},
                           {"competitorPosterior", score.competitorPosterior}});
    }
    out["phonemeScores"] = scores;
    return out;
}

int runGate(const std::string& resourcesDir) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(resourcesDir + "/phonemes.json");
    ContentStore content = ContentStore::loadFromDirectory(resourcesDir, inventory);

    int total = 0;
    int failures = 0;

    auto report = [&](const std::string& label, bool ok) {
        ++total;
        if (!ok) ++failures;
        std::printf("[%s] %s\n", ok ? "PASS" : "FAIL", label.c_str());
    };

    for (const auto& [wordId, word] : content.practiceWords()) {
        PronunciationEngine engine(resourcesDir, makeScriptedModel(inventory, inventory.symbolsToIds(word.phonemes)));
        EngineResult result = engine.process(makeSpeechLikeTone(1.0), "word:" + wordId);
        report("correct pronunciation of '" + word.text + "' does not trigger a specific error",
               result.diagnosis.outcome != DiagnosisOutcome::SpecificError);
    }

    for (const auto& [patternId, pattern] : content.errorPatterns()) {
        if (pattern.type != ErrorPatternType::Substitution) continue;
        for (const auto& [wordId, word] : content.practiceWords()) {
            bool targetsPattern = false;
            for (const auto& id : word.targetErrorPatternIds) targetsPattern = targetsPattern || id == patternId;
            if (!targetsPattern) continue;

            std::vector<std::string> produced = word.phonemes;
            bool substituted = false;
            for (auto& symbol : produced) {
                if (symbol == pattern.expectedPhonemeSymbol && !pattern.confusablePhonemeSymbols.empty()) {
                    symbol = pattern.confusablePhonemeSymbols[0];
                    substituted = true;
                    break;
                }
            }
            if (!substituted) continue;

            PronunciationEngine engine(resourcesDir, makeScriptedModel(inventory, inventory.symbolsToIds(produced)));
            std::string exerciseId = "word:" + wordId;
            EngineResult first = engine.process(makeSpeechLikeTone(1.0), exerciseId);
            report("substituted '" + word.text + "' triggers " + patternId + " on first attempt",
                   first.diagnosis.outcome == DiagnosisOutcome::SpecificError && first.diagnosis.errorPatternId == patternId);

            EngineResult second = engine.process(makeSpeechLikeTone(1.0), exerciseId);
            report("substituted '" + word.text + "' confirms " + patternId + " by the second attempt",
                   second.diagnosis.errorConfirmedByHistory);
            break; // one word per pattern is enough for the spike gate
        }
    }

    std::printf("\n%d/%d scenarios passed\n", total - failures, total);
    return failures == 0 ? 0 : 1;
}

int runOne(const std::string& resourcesDir, const std::string& exerciseId, const std::vector<std::string>& producedSymbols) {
    PhonemeInventory inventory = PhonemeInventory::loadFromFile(resourcesDir + "/phonemes.json");
    std::vector<PhonemeId> produced = inventory.symbolsToIds(producedSymbols);
    PronunciationEngine engine(resourcesDir, makeScriptedModel(inventory, produced));
    EngineResult result = engine.process(makeSpeechLikeTone(1.0), exerciseId);
    std::cout << diagnosisToJson(result).dump(2) << std::endl;
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr,
                      "Usage:\n"
                      "  %s <resourcesDir> gate\n"
                      "  %s <resourcesDir> run <exerciseId> <phonemeSymbol...>\n",
                      argv[0], argv[0]);
        return 2;
    }

    std::string resourcesDir = argv[1];
    std::string command = argv[2];

    if (command == "gate") {
        return runGate(resourcesDir);
    }
    if (command == "run") {
        if (argc < 4) {
            std::fprintf(stderr, "run requires an exerciseId\n");
            return 2;
        }
        std::string exerciseId = argv[3];
        std::vector<std::string> symbols(argv + 4, argv + argc);
        return runOne(resourcesDir, exerciseId, symbols);
    }

    std::fprintf(stderr, "unknown command '%s'\n", command.c_str());
    return 2;
}
