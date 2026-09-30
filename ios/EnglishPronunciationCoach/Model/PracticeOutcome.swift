import Foundation

/// What a stage attempt showed, kept separate on purpose: understandability,
/// the target sound, and stability are different things (spec: scoring philosophy).
struct PracticeOutcome: Equatable {
    var intelligible: Bool
    var targetSoundProduced: Bool
    var stable: Bool
}

enum StarRating {
    /// ★1 = would likely be understood, ★2 = target sound produced,
    /// ★3 = target sound also stable on a repeat. No arbitrary "accent score".
    static func stars(for outcome: PracticeOutcome) -> Int {
        guard outcome.intelligible else { return 0 }
        guard outcome.targetSoundProduced else { return 1 }
        return outcome.stable ? 3 : 2
    }
}

enum OutcomeCalculator {
    /// Builds a stage outcome from the valid (non-retry) results only.
    static func outcome(from results: [PronunciationResult]) -> PracticeOutcome {
        guard !results.isEmpty else {
            return PracticeOutcome(intelligible: false, targetSoundProduced: false, stable: false)
        }
        let intelligibleCount = results.filter { $0.intelligible }.count
        let targetCount = results.filter { $0.targetSoundProduced }.count
        let intelligible = intelligibleCount * 2 >= results.count
        let target = targetCount * 2 >= results.count
        let lastTwo = Array(results.suffix(2))
        let stable = target && lastTwo.count == 2 && lastTwo.allSatisfy { $0.targetSoundProduced }
        return PracticeOutcome(intelligible: intelligible, targetSoundProduced: target, stable: stable)
    }
}
