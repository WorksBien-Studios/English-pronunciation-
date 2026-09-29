#include "CTCAlignment.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace pronunciation {

namespace {
constexpr float kNegInf = -std::numeric_limits<float>::infinity();

bool validLogProbShape(const FrameLogProbs& logProbs) {
    if (logProbs.numFrames < 0 || logProbs.vocabSize <= 0) return false;
    if (logProbs.blankColumn < 0 || logProbs.blankColumn >= logProbs.vocabSize) return false;
    if (!(logProbs.frameDurationSeconds > 0.0) || !std::isfinite(logProbs.frameDurationSeconds)) return false;

    const size_t frames = static_cast<size_t>(logProbs.numFrames);
    const size_t vocab = static_cast<size_t>(logProbs.vocabSize);
    if (vocab != 0 && frames > std::numeric_limits<size_t>::max() / vocab) return false;
    if (logProbs.data.size() != frames * vocab) return false;

    for (float value : logProbs.data) {
        // Negative infinity is a valid log-probability for impossible states.
        if (std::isnan(value) || value == std::numeric_limits<float>::infinity()) return false;
    }
    return true;
}

bool validExpectedSequence(const FrameLogProbs& logProbs, const std::vector<PhonemeId>& expectedSequence) {
    for (PhonemeId id : expectedSequence) {
        if (id < 0 || id >= logProbs.vocabSize || id == logProbs.blankColumn) return false;
    }
    return true;
}

} // namespace

ForcedAlignmentResult forceAlign(const FrameLogProbs& logProbs, const std::vector<PhonemeId>& expectedSequence) {
    ForcedAlignmentResult result;
    const int n = static_cast<int>(expectedSequence.size());
    result.phonemes.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        result.phonemes[static_cast<size_t>(i)].expectedIndex = i;
        result.phonemes[static_cast<size_t>(i)].phonemeId = expectedSequence[static_cast<size_t>(i)];
    }

    if (!validLogProbShape(logProbs) || !validExpectedSequence(logProbs, expectedSequence)) {
        result.succeeded = false;
        return result;
    }

    const int S = 2 * n + 1;
    const int T = logProbs.numFrames;
    if (T < S) {
        result.succeeded = false;
        return result;
    }

    auto column = [&](int s) {
        return (s % 2 == 0) ? logProbs.blankColumn : expectedSequence[static_cast<size_t>((s - 1) / 2)];
    };

    std::vector<std::vector<float>> alpha(static_cast<size_t>(T), std::vector<float>(static_cast<size_t>(S), kNegInf));
    std::vector<std::vector<int8_t>> back(static_cast<size_t>(T), std::vector<int8_t>(static_cast<size_t>(S), -1));

    alpha[0][0] = logProbs.at(0, column(0));
    if (S > 1) alpha[0][1] = logProbs.at(0, column(1));

    for (int t = 1; t < T; ++t) {
        for (int s = 0; s < S; ++s) {
            float best = alpha[static_cast<size_t>(t - 1)][static_cast<size_t>(s)];
            int8_t bestOffset = 0;
            if (s - 1 >= 0 && alpha[static_cast<size_t>(t - 1)][static_cast<size_t>(s - 1)] > best) {
                best = alpha[static_cast<size_t>(t - 1)][static_cast<size_t>(s - 1)];
                bestOffset = 1;
            }
            if (s % 2 == 1 && s - 2 >= 0 && column(s) != column(s - 2) &&
                alpha[static_cast<size_t>(t - 1)][static_cast<size_t>(s - 2)] > best) {
                best = alpha[static_cast<size_t>(t - 1)][static_cast<size_t>(s - 2)];
                bestOffset = 2;
            }
            if (best == kNegInf) continue;
            alpha[static_cast<size_t>(t)][static_cast<size_t>(s)] = best + logProbs.at(t, column(s));
            back[static_cast<size_t>(t)][static_cast<size_t>(s)] = bestOffset;
        }
    }

    int endState = S - 1;
    if (S >= 2 && alpha[static_cast<size_t>(T - 1)][static_cast<size_t>(S - 2)] >
                      alpha[static_cast<size_t>(T - 1)][static_cast<size_t>(S - 1)]) {
        endState = S - 2;
    }
    result.totalLogLikelihood = alpha[static_cast<size_t>(T - 1)][static_cast<size_t>(endState)];

    if (alpha[static_cast<size_t>(T - 1)][static_cast<size_t>(endState)] == kNegInf) {
        result.succeeded = false;
        return result;
    }

    std::vector<int> path(static_cast<size_t>(T));
    int s = endState;
    for (int t = T - 1; t >= 0; --t) {
        path[static_cast<size_t>(t)] = s;
        if (t == 0) break;
        int8_t offset = back[static_cast<size_t>(t)][static_cast<size_t>(s)];
        s -= std::max<int8_t>(offset, 0);
    }

    for (int i = 0; i < n; ++i) {
        int labelState = 2 * i + 1;
        int startFrame = -1;
        int endFrame = -1;
        for (int t = 0; t < T; ++t) {
            if (path[static_cast<size_t>(t)] == labelState) {
                if (startFrame < 0) startFrame = t;
                endFrame = t;
            }
        }
        result.phonemes[static_cast<size_t>(i)].startFrame = startFrame < 0 ? 0 : startFrame;
        result.phonemes[static_cast<size_t>(i)].endFrame = startFrame < 0 ? -1 : endFrame;
    }

    result.succeeded = true;
    return result;
}

std::vector<PhonemeId> greedyDecode(const FrameLogProbs& logProbs) {
    std::vector<PhonemeId> output;
    if (!validLogProbShape(logProbs)) return output;

    int lastEmittedColumn = logProbs.blankColumn;
    for (int t = 0; t < logProbs.numFrames; ++t) {
        int argmaxColumn = 0;
        float best = kNegInf;
        for (int v = 0; v < logProbs.vocabSize; ++v) {
            float value = logProbs.at(t, v);
            if (value > best) {
                best = value;
                argmaxColumn = v;
            }
        }
        if (argmaxColumn == logProbs.blankColumn) {
            lastEmittedColumn = logProbs.blankColumn;
            continue;
        }
        if (argmaxColumn != lastEmittedColumn) output.push_back(static_cast<PhonemeId>(argmaxColumn));
        lastEmittedColumn = argmaxColumn;
    }
    return output;
}

std::vector<EditOp> levenshteinAlign(const std::vector<PhonemeId>& hypothesis, const std::vector<PhonemeId>& reference) {
    const int H = static_cast<int>(hypothesis.size());
    const int R = static_cast<int>(reference.size());
    std::vector<std::vector<int>> dp(static_cast<size_t>(H + 1), std::vector<int>(static_cast<size_t>(R + 1), 0));
    for (int h = 0; h <= H; ++h) dp[static_cast<size_t>(h)][0] = h;
    for (int r = 0; r <= R; ++r) dp[0][static_cast<size_t>(r)] = r;

    for (int h = 1; h <= H; ++h) {
        for (int r = 1; r <= R; ++r) {
            if (hypothesis[static_cast<size_t>(h - 1)] == reference[static_cast<size_t>(r - 1)]) {
                dp[static_cast<size_t>(h)][static_cast<size_t>(r)] =
                    dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r - 1)];
            } else {
                dp[static_cast<size_t>(h)][static_cast<size_t>(r)] =
                    1 + std::min({dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r - 1)],
                                  dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r)],
                                  dp[static_cast<size_t>(h)][static_cast<size_t>(r - 1)]});
            }
        }
    }

    std::vector<EditOp> ops;
    int h = H, r = R;
    while (h > 0 || r > 0) {
        if (h > 0 && r > 0 &&
            hypothesis[static_cast<size_t>(h - 1)] == reference[static_cast<size_t>(r - 1)] &&
            dp[static_cast<size_t>(h)][static_cast<size_t>(r)] ==
                dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r - 1)]) {
            ops.push_back({EditOpType::Match, h - 1, r - 1});
            --h; --r;
        } else if (h > 0 && r > 0 &&
                   dp[static_cast<size_t>(h)][static_cast<size_t>(r)] ==
                       dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r - 1)] + 1) {
            ops.push_back({EditOpType::Substitute, h - 1, r - 1});
            --h; --r;
        } else if (h > 0 &&
                   dp[static_cast<size_t>(h)][static_cast<size_t>(r)] ==
                       dp[static_cast<size_t>(h - 1)][static_cast<size_t>(r)] + 1) {
            ops.push_back({EditOpType::Insert, h - 1, -1});
            --h;
        } else {
            ops.push_back({EditOpType::Delete, -1, r - 1});
            --r;
        }
    }
    std::reverse(ops.begin(), ops.end());
    return ops;
}

} // namespace pronunciation
