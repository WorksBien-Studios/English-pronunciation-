import Foundation

/// The narrow seam between the UI and the on-device pronunciation engine
/// (shared C++ core + bundled ONNX model). The UI only depends on these types.

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
    static let recordConfidence = 0.8
    static let minObservationsToShow = 2
}

private enum NativeAudioQuality {
    static func retryReason(for request: EngineRequest) -> RetryReason? {
        guard request.sampleRate.isFinite,
              request.sampleRate > 0,
              request.samples.count <= Int(Int32.max) else {
            return .lowConfidence
        }

        let rate = Int32(request.sampleRate.rounded())
        let quality = request.samples.withUnsafeBufferPointer { buffer in
            pe_audio_quality_evaluate(
                buffer.baseAddress,
                Int32(buffer.count),
                rate
            )
        }

        guard quality.passes_gate == 0 else { return nil }
        switch Int(quality.reason.rawValue) {
        case 1: return .tooShort
        case 2: return .silence
        case 3: return .clipped
        default: return .lowConfidence
        }
    }
}

/// Production engine. Model construction and inference happen on a dedicated
/// serial queue so the ~197 MB model never blocks the main actor. The C++
/// engine owns all scoring/history state; Swift only maps its typed result.
final class NativePronunciationEngine: PronunciationEngine, @unchecked Sendable {
    private let queue = DispatchQueue(
        label: "com.worksbienstudios.englishpronunciation.engine",
        qos: .userInitiated
    )
    private let resourcesPath: String
    private let modelPath: String
    private var engine: OpaquePointer?

    init?(bundle: Bundle = .main) {
        guard let resourcesURL = bundle.resourceURL?
            .appendingPathComponent("PronunciationEngine", isDirectory: true),
              FileManager.default.fileExists(atPath: resourcesURL.path) else {
            return nil
        }

        let modelURL = resourcesURL.appendingPathComponent("model_q4f16.onnx")
        guard FileManager.default.fileExists(atPath: modelURL.path) else {
            return nil
        }

        resourcesPath = resourcesURL.path
        modelPath = modelURL.path
    }

    deinit {
        if let engine {
            pe_engine_destroy(engine)
        }
    }

    func analyze(_ request: EngineRequest) async -> EngineDecision {
        // This uses the shared C++ quality gate and deliberately runs before
        // model construction, preserving useful feedback even if ORT fails.
        if let reason = NativeAudioQuality.retryReason(for: request) {
            return .retry(reason)
        }

        return await withCheckedContinuation { continuation in
            queue.async { [self] in
                continuation.resume(returning: analyzeAccepted(request))
            }
        }
    }

    private func analyzeAccepted(_ request: EngineRequest) -> EngineDecision {
        guard let engine = ensureEngine() else {
            return .retry(.engineUnavailable)
        }

        guard request.samples.count <= Int(Int32.max),
              request.sampleRate.isFinite,
              request.sampleRate > 0 else {
            return .retry(.lowConfidence)
        }

        var errorMessage: UnsafeMutablePointer<CChar>?
        let exerciseID = "word:" + request.word.text.lowercased()
        let result: OpaquePointer? = request.samples.withUnsafeBufferPointer { buffer in
            exerciseID.withCString { exerciseCString in
                pe_engine_process(
                    engine,
                    buffer.baseAddress,
                    Int32(buffer.count),
                    Int32(request.sampleRate.rounded()),
                    exerciseCString,
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

        let quality = pe_result_audio_quality(result)
        if quality.passes_gate == 0 {
            switch Int(quality.reason.rawValue) {
            case 1: return .retry(.tooShort)
            case 2: return .retry(.silence)
            case 3: return .retry(.clipped)
            default: return .retry(.lowConfidence)
            }
        }

        let diagnosis = pe_result_diagnosis(result)
        let confidence: Double
        switch Int(diagnosis.confidence.rawValue) {
        case 2: confidence = 0.9
        case 1: confidence = 0.7
        default: confidence = 0.4
        }

        switch Int(diagnosis.outcome.rawValue) {
        case 0:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: true,
                    likelySubstitution: nil,
                    confidence: confidence
                )
            )
        case 2:
            return .result(
                PronunciationResult(
                    intelligible: true,
                    targetSoundProduced: false,
                    likelySubstitution: request.word.contrast,
                    confidence: confidence
                )
            )
        default:
            return .retry(.lowConfidence)
        }
    }

    private func ensureEngine() -> OpaquePointer? {
        if let engine { return engine }

        var errorMessage: UnsafeMutablePointer<CChar>?
        let created: OpaquePointer? = resourcesPath.withCString { resourcesCString in
            modelPath.withCString { modelCString in
                pe_engine_create(resourcesCString, modelCString, &errorMessage)
            }
        }

        if let errorMessage {
            pe_free_error_message(errorMessage)
        }
        engine = created
        return created
    }
}

/// Fallback when the signed app bundle genuinely lacks a usable production
/// engine. It still returns the shared, real audio-quality feedback first.
struct UnavailablePronunciationEngine: PronunciationEngine {
    func analyze(_ request: EngineRequest) async -> EngineDecision {
        if let reason = NativeAudioQuality.retryReason(for: request) {
            return .retry(reason)
        }
        return .retry(.engineUnavailable)
    }
}

/// Explicit preview/test double only. EngineFactory never selects this.
struct PreviewPronunciationEngine: PronunciationEngine {
    func analyze(_ request: EngineRequest) async -> EngineDecision {
        guard request.samples.count > Int(request.sampleRate / 4) else {
            return .retry(.tooShort)
        }
        let energy = request.samples.reduce(Float(0)) { $0 + $1 * $1 } /
            Float(request.samples.count)
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
    static func makeDefault(bundle: Bundle = .main) -> PronunciationEngine {
        NativePronunciationEngine(bundle: bundle) ??
            UnavailablePronunciationEngine()
    }
}
