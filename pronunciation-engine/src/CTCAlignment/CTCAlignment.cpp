#include "CTCAlignment.h"

#include <algorithm>
#include <limits>

namespace pronunciation {

namespace {
constexpr float kNegInf = -std::numeric_limits<float>::infinity();
}

ForcedAlignmentResult forceAlign(const FrameLogProbs& logProbs, const std::vector<PhonemeId>& expectedSequence) {
    ForcedAlignmentResult result;
    const int n = static_cast<int>(expectedSequence.size());
    const int S = 2 * n + 1;
    const int T = logProbs.numFrames;

    result.phonemes.resize(n);
    for (int i = 0; i < n; ++i) {
        result.phonemes[i].expectedIndex = i;
        result.phonemes[i].phonemeId = expectedSequence[i];
    }

    if (T < S) {
        result.succeeded = false;
        return result;
    }

    auto column = [&](int s) { return (s % 2 == 0) ? logProbs.blankColumn : expectedSequence[(s - 1) / 2]; };

    std::vector<std::vector<float>> alpha(T, std::vector<float>(S, kNegInf));
    std::vector<std::vector<int8_t>> back(T, std::vector<int8_t>(S, -1));

    alpha[0][0] = logProbs.at(0, column(0));
    if (S > 1) alpha[0][1] = logProbs.at(0, column(1));

    for (int t = 1; t < T; ++t) {
        for (int s = 0; s < S; ++s) {
            float best = alpha[t - 1][s];
            int8_t bestOffset = 0;
            if (s - 1 >= 0 && alpha[t - 1][s - 1] > best) {
                best = alpha[t - 1][s - 1];
                bestOffset = 1;
            }
            // A skip transition (offset 2) only ever lands on a label state
            // (odd s) and only when that label differs from the one two
            // states back, i.e. there is no required blank between two
            // distinct consecutive phonemes.
            if (s % 2 == 1 && s - 2 >= 0 && column(s) != column(s - 2) && alpha[t - 1][s - 2] > best) {
                best = alpha[t - 1][s - 2];
                bestOffset = 2;
            }
            if (best == kNegInf) continue;
            alpha[t][s] = best + logProbs.at(t, column(s));
            back[t][s] = bestOffset;
        }
    }

    int endState = S - 1;
    if (S >= 2 && alpha[T - 1][S - 2] > alpha[T - 1][S - 1]) endState = S - 2;
    result.totalLogLikelihood = alpha[T - 1][endState];

    if (alpha[T - 1][endState] == kNegInf) {
        result.succeeded = false;
        return result;
    }

    std::vector<int> path(T);
    int s = endState;
    for (int t = T - 1; t >= 0; --t) {
        path[t] = s;
        if (t == 0) break;
        int8_t offset = back[t][s];
        s -= std::max<int8_t>(offset, 0);
    }

    for (int i = 0; i < n; ++i) {
        int labelState = 2 * i + 1;
        int startFrame = -1;
        int endFrame = -1;
        for (int t = 0; t < T; ++t) {
            if (path[t] == labelState) {
                if (startFrame < 0) startFrame = t;
                endFrame = t;
            }
        }
        result.phonemes[i].startFrame = startFrame < 0 ? 0 : startFrame;
        result.phonemes[i].endFrame = startFrame < 0 ? -1 : endFrame;
    }

    result.succeeded = true;
    return result;
}

std::vector<PhonemeId> greedyDecode(const FrameLogProbs& logProbs) {
    std::vector<PhonemeId> output;
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
        if (argmaxColumn != lastEmittedColumn) {
            output.push_back(static_cast<PhonemeId>(argmaxColumn));
        }
        lastEmittedColumn = argmaxColumn;
    }
    return output;
}

std::vector<EditOp> levenshteinAlign(const std::vector<PhonemeId>& hypothesis, const std::vector<PhonemeId>& reference) {
    const int H = static_cast<int>(hypothesis.size());
    const int R = static_cast<int>(reference.size());
    std::vector<std::vector<int>> dp(H + 1, std::vector<int>(R + 1, 0));
    for (int h = 0; h <= H; ++h) dp[h][0] = h;
    for (int r = 0; r <= R; ++r) dp[0][r] = r;

    for (int h = 1; h <= H; ++h) {
        for (int r = 1; r <= R; ++r) {
            if (hypothesis[h - 1] == reference[r - 1]) {
                dp[h][r] = dp[h - 1][r - 1];
            } else {
                dp[h][r] = 1 + std::min({dp[h - 1][r - 1], dp[h - 1][r], dp[h][r - 1]});
            }
        }
    }

    std::vector<EditOp> ops;
    int h = H, r = R;
    while (h > 0 || r > 0) {
        if (h > 0 && r > 0 && hypothesis[h - 1] == reference[r - 1] && dp[h][r] == dp[h - 1][r - 1]) {
            ops.push_back({EditOpType::Match, h - 1, r - 1});
            --h;
            --r;
        } else if (h > 0 && r > 0 && dp[h][r] == dp[h - 1][r - 1] + 1) {
            ops.push_back({EditOpType::Substitute, h - 1, r - 1});
            --h;
            --r;
        } else if (h > 0 && dp[h][r] == dp[h - 1][r] + 1) {
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
