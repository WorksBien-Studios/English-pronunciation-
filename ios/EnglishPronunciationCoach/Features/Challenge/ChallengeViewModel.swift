import Foundation
import Observation

/// Drives one stage attempt: word → record → analyse → feedback → next.
@MainActor
@Observable
final class ChallengeViewModel {
    enum Phase: Equatable {
        case ready
        case recording
        case analyzing
        case feedback(EngineDecision)
        case finished
    }

    let stage: Stage
    let recorder = AudioRecorder()

    private(set) var index = 0
    private(set) var phase: Phase = .ready
    /// Only results that were not retries; these are what counts toward stars and the daily allowance.
    private(set) var validResults: [PronunciationResult] = []

    @ObservationIgnored private let engine: PronunciationEngine
    @ObservationIgnored private let player = ModelAudioPlayer()

    init(stage: Stage, engine: PronunciationEngine) {
        self.stage = stage
        self.engine = engine
    }

    var word: PracticeWord { stage.words[min(index, stage.words.count - 1)] }
    var progressText: String { "\(index + 1)/\(stage.words.count)" }
    var isLastWord: Bool { index + 1 >= stage.words.count }

    func playModel(slow: Bool) {
        player.speak(word.text, slow: slow)
    }

    func startRecording() async {
        guard phase == .ready else { return }
        await recorder.start()
        if recorder.state == .recording {
            phase = .recording
        }
    }

    /// Stops the recording, asks the engine, and reports valid results to `onValid`.
    func stopAndAnalyze(onValid: (PronunciationResult) -> Void) async {
        guard phase == .recording else { return }
        let samples = recorder.stop()
        phase = .analyzing
        let request = EngineRequest(word: word, samples: samples, sampleRate: recorder.sampleRate)
        let decision = await engine.analyze(request)
        if case .result(let result) = decision {
            validResults.append(result)
            onValid(result)
        }
        phase = .feedback(decision)
    }

    /// Retrying never costs anything unless the previous attempt produced a valid result.
    func retry() {
        phase = .ready
    }

    func advance() {
        if isLastWord {
            phase = .finished
        } else {
            index += 1
            phase = .ready
        }
    }

    var outcome: PracticeOutcome {
        OutcomeCalculator.outcome(from: validResults)
    }
}
