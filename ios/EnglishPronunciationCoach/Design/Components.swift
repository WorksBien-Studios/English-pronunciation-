import SwiftUI

/// The four articulation cues (舌 / 唇 / 息 / 声) as a native grouped list.
struct CueList: View {
    let cues: [ArticulationCue]

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("発音のポイント")
                .font(.headline)
                .padding(.horizontal, 16)
                .padding(.top, 14)
                .padding(.bottom, 4)
            ForEach(cues) { cue in
                HStack(spacing: 12) {
                    Text(cue.part)
                        .font(.title3.bold())
                        .foregroundStyle(.white)
                        .frame(width: 36, height: 36)
                        .background(Color.brand, in: RoundedRectangle(cornerRadius: 10, style: .continuous))
                        .accessibilityHidden(true)
                    Text(cue.instruction)
                        .font(.body)
                        .frame(maxWidth: .infinity, alignment: .leading)
                }
                .padding(.horizontal, 16)
                .padding(.vertical, 10)
                .accessibilityElement(children: .combine)
                .accessibilityLabel("\(cue.part)、\(cue.instruction)")
                if cue.id != cues.last?.id {
                    Divider().padding(.leading, 64)
                }
            }
        }
        .padding(.bottom, 6)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
    }
}

/// One practice word with model-audio buttons.
@MainActor
struct WordRow: View {
    let word: PracticeWord
    let player: ModelAudioPlayer

    var body: some View {
        HStack(spacing: 10) {
            VStack(alignment: .leading, spacing: 0) {
                Text(word.text).font(.title3.weight(.semibold))
                Text(word.ipa).font(.footnote).foregroundStyle(.secondary)
            }
            Spacer(minLength: 8)
            Button {
                player.speak(word.text, slow: false)
            } label: {
                Label("お手本", systemImage: "speaker.wave.2.fill")
            }
            Button {
                player.speak(word.text, slow: true)
            } label: {
                Label("ゆっくり", systemImage: "tortoise.fill")
            }
        }
        .font(.footnote)
        .buttonStyle(.bordered)
        .controlSize(.small)
    }
}

/// Live input level as a row of bars. The bars scroll left as new levels arrive.
struct LiveWaveform: View {
    let level: Float
    var barCount = 48

    @State private var levels: [Float] = []

    var body: some View {
        let bars = levels.isEmpty ? Array(repeating: Float(0), count: barCount) : levels
        let count = barCount
        return Canvas { context, size in
            let slot = size.width / CGFloat(count)
            for (index, value) in bars.enumerated() {
                let height = max(4, CGFloat(value) * size.height)
                let rect = CGRect(
                    x: CGFloat(index) * slot + slot * 0.2,
                    y: (size.height - height) / 2,
                    width: slot * 0.6,
                    height: height
                )
                context.fill(Path(roundedRect: rect, cornerRadius: rect.width / 2), with: .foreground)
            }
        }
        .frame(height: 72)
        .foregroundStyle(Color.brand)
        .onChange(of: level) { _, newValue in
            var updated = levels.isEmpty ? Array(repeating: Float(0), count: barCount) : levels
            updated.append(newValue)
            levels = Array(updated.suffix(barCount))
        }
        .accessibilityHidden(true)
    }
}

struct ProBadge: View {
    var body: some View {
        Text("Pro")
            .font(.caption2.bold())
            .foregroundStyle(.secondary)
            .padding(.horizontal, 8)
            .padding(.vertical, 2)
            .background(Color(.tertiarySystemFill), in: Capsule())
    }
}
