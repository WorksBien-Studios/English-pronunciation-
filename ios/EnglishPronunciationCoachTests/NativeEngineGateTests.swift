import XCTest
@testable import EnglishPronunciationCoach

/// Behaviour of the production adapter that does not depend on real speech: the audio-quality gate runs before
/// any model is loaded, and every failure mode must come back as a retry, never as a fabricated score.
final class NativeEngineGateTests: XCTestCase {
    private let word = PracticeWord(text: "right", ipa: "/raɪt/", target: .r, contrast: .l)

    /// A quiet lead-in followed by a tone, like a recording that starts before the speaker does.
    private func speechLikeSamples(seconds: Double, sampleRate: Double, amplitude: Double = 0.35) -> [Float] {
        var samples = [Float](repeating: 0, count: Int(sampleRate * seconds))
        let leadIn = samples.count / 6
        for index in leadIn..<samples.count {
            samples[index] = Float(amplitude * sin(2 * Double.pi * 220 * Double(index) / sampleRate))
        }
        return samples
    }

    private func analyze(_ samples: [Float], sampleRate: Double = 16_000, word: PracticeWord? = nil) async -> EngineDecision {
        await NativePronunciationEngine().analyze(
            EngineRequest(word: word ?? self.word, samples: samples, sampleRate: sampleRate)
        )
    }

    func testSilenceIsRetriedAtDeviceAndEngineRates() async {
        let rates: [Double] = [16_000, 44_100, 48_000]
        for rate in rates {
            let decision = await analyze([Float](repeating: 0, count: Int(rate)), sampleRate: rate)
            XCTAssertEqual(decision, .retry(.silence), "rate \(rate)")
        }
    }

    func testTooShortRecordingIsRetried() async {
        let decision = await analyze(speechLikeSamples(seconds: 0.1, sampleRate: 16_000))
        XCTAssertEqual(decision, .retry(.tooShort))
    }

    func testClippedRecordingIsRetried() async {
        let square = (0..<16_000).map { Float($0 / 40 % 2 == 0 ? 1 : -1) }
        let decision = await analyze(square)
        XCTAssertEqual(decision, .retry(.clipped))
    }

    func testInvalidSampleRatesAreRetriedWithoutCallingTheEngine() async {
        let samples = speechLikeSamples(seconds: 1, sampleRate: 16_000)
        let invalidRates: [Double] = [0, -16_000, .nan, .infinity]
        for rate in invalidRates {
            let decision = await analyze(samples, sampleRate: rate)
            XCTAssertEqual(decision, .retry(.tooShort), "rate \(rate)")
        }
    }

    func testNonFiniteSamplesNeverProduceAResult() async {
        var samples = speechLikeSamples(seconds: 1, sampleRate: 16_000)
        samples[8_000] = .nan
        samples[8_001] = .infinity
        let decision = await analyze(samples)
        guard case .retry = decision else {
            return XCTFail("Non-finite audio must be retried, got \(decision)")
        }
    }

    func testUnknownWordIsUnavailableNotScored() async {
        let unknown = PracticeWord(text: "zzzunknownword", ipa: "/z/", target: .r, contrast: .l)
        let decision = await analyze(speechLikeSamples(seconds: 1.5, sampleRate: 16_000), word: unknown)
        XCTAssertEqual(decision, .retry(.engineUnavailable))
    }

    func testDeviceRateAudioReachesInferenceThroughTheResampler() async {
        // 48 kHz is what the microphone delivers; the C++ backend resamples to the model's 16 kHz.
        // A synthetic tone is not a pronunciation, so a conservative low-confidence retry is fine;
        // an unavailable engine or a gate rejection means the path is broken.
        let decision = await analyze(speechLikeSamples(seconds: 1.5, sampleRate: 48_000), sampleRate: 48_000)
        switch decision {
        case .result, .retry(.lowConfidence):
            break
        case .retry(let reason):
            XCTFail("48 kHz audio did not reach inference: \(reason)")
        }
    }
}
