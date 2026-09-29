import Foundation
import PronunciationEngineCore

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

/// Production engine. The C++ core owns audio-quality gating, ONNX inference,
/// forced alignment, scoring and conservative decision rules. The model and
/// engine content are local bundle resources; no audio leaves the device.
actor NativePronunciationEngine: PronunciationEngine {
    private var handle: OpaquePointer?
    private var attemptedInitialization = false

    deinit {
        if let handle {
            pe_engine_destroy(handle)
        }
    }

    func analyze(_ request: EngineRequest) async -> EngineDecision {
        guard request.sampleRate.isFinite,
              request.sampleRate > 0,
              request.samples.count <= Int(Int32.max),
              let engine = engineHandle() else {
            return .retry(.engineUnavailable)
        }

        let exerciseID = "word:" + request.word.text.lowercased()
        var errorMessage: UnsafeMutablePointer<CChar>?
        let result: OpaquePointer? = request.samples.withUnsafeBufferPointer { samples in
            exerciseID.withCString { exercise in
                pe_engine_process(
                    engine,
                    samples.baseAddress,
                    Int32(samples.count),
                    Int32(request.sampleRate.rounded()),
                    exercise,
                    &errorMessage
                )
            }
        }

        if let errorMessage {
            pe_free_error_message(errorMessage)
        }
        guard let result else {
            return .retry(.engineUnavailable)
        }
        defer { pe_engine_result_free(result) }

        let audio = pe_result_audio_quality(result)
        if audio.passes_gate == 0 {
            // pe_audio_quality_reason: OK=0, TOO_SHORT=1, SILENCE=2,
            // CLIPPING=3, LOW_SNR=4.
            switch Int(audio.reason.rawValue) {
            case 1: return .retry(.tooShort)
            case 2: return .retry(.silence)
            case 3: return .retry(.clipped)
            default: return .retry(.lowConfidence)
            }
        }

        let diagnosis = pe_result_diagnosis(result)
        // pe_diagnosis_outcome: PASS=0, RETRY=1, SPECIFIC_ERROR=2.
        switch Int(diagnosis.outcome.rawValue) {
        case 0:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: true,
                    likelySubstitution: nil,
                    confidence: confidenceValue(diagnosis.confidence)
                )
            )
        case 2:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: false,
                    likelySubstitution: sound(forInternalPhonemeID: Int(diagnosis.produced_phoneme_id)),
                    confidence: confidenceValue(diagnosis.confidence)
                )
            )
        default:
            return .retry(.lowConfidence)
        }
    }

    private func engineHandle() -> OpaquePointer? {
        if attemptedInitialization {
            return handle
        }
        attemptedInitialization = true

        guard let resourceRoot = Bundle.main.resourceURL?
            .appendingPathComponent("PronunciationEngineResources", isDirectory: true) else {
            return nil
        }
        let modelURL = resourceRoot.appendingPathComponent("model_q4f16.onnx")
        guard FileManager.default.fileExists(atPath: modelURL.path),
              FileManager.default.fileExists(
                atPath: resourceRoot.appendingPathComponent("phonemes.json").path
              ) else {
            return nil
        }

        var errorMessage: UnsafeMutablePointer<CChar>?
        handle = resourceRoot.path.withCString { resources in
            modelURL.path.withCString { model in
                pe_engine_create(resources, model, &errorMessage)
            }
        }
        if let errorMessage {
            pe_free_error_message(errorMessage)
        }
        return handle
    }

    private func confidenceValue(_ confidence: pe_confidence_level) -> Double {
        // pe_confidence_level: LOW=0, MEDIUM=1, HIGH=2.
        switch Int(confidence.rawValue) {
        case 2: return 0.9
        case 1: return 0.65
        default: return 0.35
        }
    }

    private func sound(forInternalPhonemeID id: Int) -> Sound? {
        // Stable IDs from pronunciation-engine/Resources/phonemes.json.
        switch id {
        case 16: return .b
        case 20: return .f
        case 25: return .l
        case 30: return .r
        case 34: return .th
        case 35: return .v
        default: return nil
        }
    }
}

/// Explicit fail-closed implementation retained for tests and contingency UI.
struct UnavailablePronunciationEngine: PronunciationEngine {
    func analyze(_ request: EngineRequest) async -> EngineDecision {
        .retry(.engineUnavailable)
    }
}

/// Debug/preview-only deterministic fake. Production/default construction never
/// selects this implementation.
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
        NativePronunciationEngine()
    }
}
