#pragma once

#include <vector>

#include "../Types.h"

namespace pronunciation {

// Pipeline step 4 (docs/product-specification.md#model-strategy): force-align
// frame-level phoneme log-probabilities against the exercise's prevalidated
// expected phoneme sequence. This is the standard CTC forced-alignment
// (a.k.a. CTC segmentation) Viterbi search over the extended label sequence
// [blank, l1, blank, l2, ..., ln, blank]: every expected label state must be
// visited (labels cannot be skipped), so the aligner always assigns a frame
// span to every expected phoneme. Whether that span's acoustic evidence
// actually supports the expected phoneme is left to PhonemeScoring — an
// empty/never-supported span is how a deletion surfaces downstream, not a
// missing entry here.
//
// Returns succeeded=false only when there are fewer frames than the
// expected sequence requires (numFrames < 2*n+1), which the caller should
// treat as insufficient evidence (never a definitive diagnosis).
ForcedAlignmentResult forceAlign(const FrameLogProbs& logProbs, const std::vector<PhonemeId>& expectedSequence);

// Unconstrained CTC greedy decode (argmax per frame, collapse repeats,
// drop blanks). Used alongside forceAlign(), never instead of it, to detect
// phonemes the learner produced that are not in the expected sequence at
// all (insertions) — something a forced alignment constrained to the
// expected sequence cannot represent by construction.
std::vector<PhonemeId> greedyDecode(const FrameLogProbs& logProbs);

enum class EditOpType {
    Match,
    Substitute,
    Insert, // present in hypothesis, absent from reference
    Delete, // present in reference, absent from hypothesis
};

struct EditOp {
    EditOpType type;
    int hypothesisIndex = -1; // -1 for Delete
    int referenceIndex = -1;  // -1 for Insert
};

// Minimum-edit-distance alignment between a free decode (hypothesis) and
// the exercise's expected sequence (reference), used by DecisionRules to
// flag exercise-level insertion/deletion error patterns (e.g. the
// epenthetic vowel Japanese speakers add after a word-final consonant).
std::vector<EditOp> levenshteinAlign(const std::vector<PhonemeId>& hypothesis, const std::vector<PhonemeId>& reference);

} // namespace pronunciation
