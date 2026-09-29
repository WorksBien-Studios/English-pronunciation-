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

/// Owns the C engine pointer and frees it when released. The pointer is only ever used from
/// `NativePronunciationEngine`, which serialises access, so handing this box across isolation domains is safe.
private final class EngineHandle: @unchecked Sendable {
    let pointer: OpaquePointer

    init(_ pointer: OpaquePointer) {
        self.pointer = pointer
    }

    deinit {
        pe_engine_destroy(pointer)
    }
}

/// Production adapter for the C++ core and its ONNX Runtime backend. The
/// actor serializes access to the engine's error-history state and loads the
/// large model lazily on the first usable recording.
actor NativePronunciationEngine: PronunciationEngine {
    private var handle: EngineHandle?
    private var attemptedInitialization = false

    func analyze(_ request: EngineRequest) async -> EngineDecision {
        guard request.sampleRate.isFinite,
              request.sampleRate > 0,
              request.sampleRate <= Double(Int32.max),
              request.samples.count <= Int(Int32.max) else {
            return .retry(.tooShort)
        }

        let sampleRate = Int32(request.sampleRate.rounded())
        let quality = request.samples.withUnsafeBufferPointer { buffer in
            pe_check_audio_quality(buffer.baseAddress, Int32(buffer.count), sampleRate)
        }
        guard quality.passes_gate != 0 else {
            switch quality.reason {
            case PE_AUDIO_TOO_SHORT: return .retry(.tooShort)
            case PE_AUDIO_SILENCE: return .retry(.silence)
            case PE_AUDIO_CLIPPING: return .retry(.clipped)
            default: return .retry(.lowConfidence)
            }
        }

        guard let engine = loadEngineIfNeeded() else {
            return .retry(.engineUnavailable)
        }

        var errorMessage: UnsafeMutablePointer<CChar>?
        let exerciseID = "word:\(request.word.id.lowercased())"
        let result = request.samples.withUnsafeBufferPointer { buffer in
            exerciseID.withCString { exercise in
                pe_engine_process(
                    engine,
                    buffer.baseAddress,
                    Int32(buffer.count),
                    sampleRate,
                    exercise,
                    &errorMessage
                )
            }
        }
        if let errorMessage { pe_free_error_message(errorMessage) }
        guard let result else { return .retry(.engineUnavailable) }
        defer { pe_engine_result_free(result) }

        let diagnosis = pe_result_diagnosis(result)
        switch diagnosis.outcome {
        case PE_OUTCOME_PASS:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: true,
                    likelySubstitution: nil,
                    confidence: confidenceValue(diagnosis.confidence)
                )
            )
        case PE_OUTCOME_SPECIFIC_ERROR:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: false,
                    likelySubstitution: request.word.contrast,
                    confidence: confidenceValue(diagnosis.confidence)
                )
            )
        default:
            return .retry(.lowConfidence)
        }
    }

    private func loadEngineIfNeeded() -> OpaquePointer? {
        if let handle { return handle.pointer }
        guard !attemptedInitialization else { return nil }
        attemptedInitialization = true

        guard let resourceRoot = Bundle.main.resourceURL?
                .appendingPathComponent("PronunciationEngineData", isDirectory: true),
              FileManager.default.fileExists(
                atPath: resourceRoot.appendingPathComponent("phonemes.json").path),
              let modelURL = Bundle.main.url(
                forResource: "model_q4f16", withExtension: "onnx") else {
            return nil
        }

        var errorMessage: UnsafeMutablePointer<CChar>?
        let created = resourceRoot.path.withCString { resources in
            modelURL.path.withCString { model in
                pe_engine_create(resources, model, &errorMessage)
            }
        }
        if let errorMessage { pe_free_error_message(errorMessage) }
        guard let created else { return nil }
        handle = EngineHandle(created)
        return created
    }

    private func confidenceValue(_ confidence: pe_confidence_level) -> Double {
        switch confidence {
        case PE_CONFIDENCE_HIGH: return 0.95
        case PE_CONFIDENCE_MEDIUM: return 0.7
        default: return 0.4
        }
    }
}

enum EngineFactory {
    private static let productionEngine: PronunciationEngine = NativePronunciationEngine()

    static func makeDefault() -> PronunciationEngine {
        productionEngine
    }
}
