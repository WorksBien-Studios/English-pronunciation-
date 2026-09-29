import AVFoundation
import Foundation
import Observation

/// Thread-safe in-memory sample store written from the audio thread.
final class SampleBuffer: @unchecked Sendable {
    private let lock = NSLock()
    private var storage: [Float] = []

    func reset() {
        lock.lock()
        storage.removeAll(keepingCapacity: true)
        lock.unlock()
    }

    func append(_ pointer: UnsafeBufferPointer<Float>) {
        lock.lock()
        storage.append(contentsOf: pointer)
        lock.unlock()
    }

    func snapshot() -> [Float] {
        lock.lock()
        defer { lock.unlock() }
        return storage
    }
}

/// Microphone capture. Audio stays in memory and is discarded after analysis;
/// nothing is written to disk or sent over the network.
@MainActor
@Observable
final class AudioRecorder {
    enum State: Equatable {
        case idle, requestingPermission, denied, recording, failed
    }

    private(set) var state: State = .idle
    /// Smoothed input level, 0...1, for the waveform.
    private(set) var level: Float = 0
    private(set) var sampleRate: Double = 16_000

    @ObservationIgnored private let engine = AVAudioEngine()
    @ObservationIgnored private let buffer = SampleBuffer()

    /// Requests permission just in time (first recording), then starts capture.
    func start() async {
        state = .requestingPermission
        let granted = await AVAudioApplication.requestRecordPermission()
        guard granted else {
            state = .denied
            return
        }
        do {
            let session = AVAudioSession.sharedInstance()
            try session.setCategory(.record, mode: .measurement, options: [])
            try session.setActive(true)

            let input = engine.inputNode
            let format = input.outputFormat(forBus: 0)
            sampleRate = format.sampleRate
            buffer.reset()
            input.installTap(
                onBus: 0,
                bufferSize: 2048,
                format: format,
                block: Self.makeTapHandler(buffer: buffer) { [weak self] value in
                    Task { @MainActor in self?.level = value }
                }
            )
            engine.prepare()
            try engine.start()
            state = .recording
        } catch {
            state = .failed
        }
    }

    /// Stops capture and returns the recorded samples.
    func stop() -> [Float] {
        guard state == .recording else { return [] }
        engine.inputNode.removeTap(onBus: 0)
        engine.stop()
        try? AVAudioSession.sharedInstance().setActive(false, options: .notifyOthersOnDeactivation)
        state = .idle
        level = 0
        return buffer.snapshot()
    }

    /// Built in a nonisolated context so the closure does not inherit main-actor isolation
    /// (it runs on the realtime audio thread).
    nonisolated private static func makeTapHandler(
        buffer: SampleBuffer,
        onLevel: @escaping @Sendable (Float) -> Void
    ) -> AVAudioNodeTapBlock {
        return { pcm, _ in
            guard let channel = pcm.floatChannelData?[0] else { return }
            let frames = Int(pcm.frameLength)
            let pointer = UnsafeBufferPointer(start: channel, count: frames)
            buffer.append(pointer)
            var sum: Float = 0
            for sample in pointer { sum += sample * sample }
            let rms = frames > 0 ? (sum / Float(frames)).squareRoot() : 0
            onLevel(min(1, rms * 8))
        }
    }
}
