import Foundation

/// The narrow seam between the UI and the on-device pronunciation engine
/// (C++ core + ONNX model, see docs/product-specification.md). The UI only
/// depends on these types; the engine repository contains no UI code.

struct EngineRequest: Sendable {
    let word: PracticeWord
    /// Mono PCM samples, kept in memory only and discarded after analysis.
    let samples: [Float]
    let sampleRate: Double
}

struct PronunciationResult: Equatable, Sendable {
    let intelligible: Bool
    let targetSoundProduced: Bool
    let likelySubstitution: Sound?
    /// 0...1. Low confidence must come back as `.retry`, never as a definitive result.
    let confidence: Double
}

enum RetryReason: Equatable, Sendable {
    case silence
    case clipped
    case tooShort
    case lowConfidence
    case engineUnavailable
}

enum EngineDecision: Equatable, Sendable {
    case retry(RetryReason)
    case result(PronunciationResult)
}

protocol PronunciationEngine: Sendable {
    func analyze(_ request: EngineRequest) async -> EngineDecision
}

enum EngineThresholds {
    /// A specific error is only recorded in the history above this confidence.
    static let recordConfidence = 0.8
    /// The same pattern must be seen at least this many times before it is shown.
    static let minObservationsToShow = 2
}

/// Release default until the real engine is linked: honest, never fabricates a score.
struct UnavailablePronunciationEngine: PronunciationEngine {
    func analyze(_ request: EngineRequest) async -> EngineDecision {
        .retry(.engineUnavailable)
    }
}

/// Debug/preview only. Returns a fixed plausible result so the UI flow can be exercised.
/// It is never used in Release builds (see `EngineFactory`).
struct PreviewPronunciationEngine: PronunciationEngine {
    func analyze(_ request: EngineRequest) async -> EngineDecision {
        guard request.samples.count > Int(request.sampleRate / 4) else { return .retry(.tooShort) }
        let energy = request.samples.reduce(Float(0)) { $0 + $1 * $1 } / Float(request.samples.count)
        guard energy > 0.000_001 else { return .retry(.silence) }
        return .result(
            PronunciationResult(
                intelligible: true,
                targetSoundProduced: request.word.text.count % 2 == 0,
                likelySubstitution: request.word.contrast,
                confidence: 0.9
            )
        )
    }
}

enum EngineFactory {
    static func makeDefault() -> PronunciationEngine {
        #if DEBUG
        return PreviewPronunciationEngine()
        #else
        return UnavailablePronunciationEngine()
        #endif
    }
}
