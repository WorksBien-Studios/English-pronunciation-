import AVFoundation
import Foundation

/// Model pronunciation uses iOS speech synthesis (AVSpeechSynthesizer). It is labelled
/// as such in the UI and is never presented as a native-speaker recording.
@MainActor
final class ModelAudioPlayer {
    private let synthesizer = AVSpeechSynthesizer()

    func speak(_ text: String, slow: Bool) {
        try? AVAudioSession.sharedInstance().setCategory(.playback, mode: .spokenAudio)
        try? AVAudioSession.sharedInstance().setActive(true)
        synthesizer.stopSpeaking(at: .immediate)
        let utterance = AVSpeechUtterance(string: text)
        utterance.voice = AVSpeechSynthesisVoice(language: "en-US")
        let base = AVSpeechUtteranceDefaultSpeechRate
        utterance.rate = slow ? base * 0.6 : base
        synthesizer.speak(utterance)
    }
}
