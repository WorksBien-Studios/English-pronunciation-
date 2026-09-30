import SwiftUI

/// One stage: the sounds, how to make them, the practice words, and the challenge entry point.
@MainActor
struct StageDetailView: View {
    let stage: Stage

    @State private var challengeStage: Stage?
    @State private var player = ModelAudioPlayer()

    var body: some View {
        GeometryReader { geometry in
            ScrollView {
                if geometry.size.width >= 560 {
                    HStack(alignment: .top, spacing: 16) {
                        VStack(spacing: 16) {
                            soundsCard
                            wordsCard
                        }
                        CueList(cues: stage.cues)
                    }
                    .padding(16)
                } else {
                    VStack(spacing: 16) {
                        soundsCard
                        CueList(cues: stage.cues)
                        wordsCard
                    }
                    .padding(16)
                }
            }
        }
        .background(Color(.systemGroupedBackground))
        .safeAreaInset(edge: .bottom) {
            Button {
                challengeStage = stage
            } label: {
                Label("録音チャレンジをはじめる", systemImage: "mic.fill")
                    .frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
            .padding(.horizontal, 16)
            .padding(.vertical, 10)
            .frame(maxWidth: Layout.readableWidth + 32)
            .frame(maxWidth: .infinity)
            .background(.bar)
        }
        .navigationTitle(stage.title)
        .navigationBarTitleDisplayMode(.inline)
        .fullScreenCover(item: $challengeStage) { current in
            ChallengeView(stage: current) { challengeStage = nil }
        }
    }

    // MARK: Cards

    private var soundsCard: some View {
        VStack(spacing: 12) {
            HStack(spacing: 24) {
                ForEach(Array(stage.sounds.enumerated()), id: \.element.id) { index, sound in
                    if index > 0 {
                        Image(systemName: "arrow.left.arrow.right")
                            .font(.title3)
                            .foregroundStyle(.tertiary)
                            .accessibilityHidden(true)
                    }
                    SoundGlyph(sound: sound)
                }
            }
            Text(stage.summary)
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
        }
        .padding(16)
        .frame(maxWidth: .infinity)
        .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
    }

    private var wordsCard: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("練習する単語").font(.headline)
            ForEach(stage.words) { word in
                WordRow(word: word, player: player)
                if word.id != stage.words.last?.id { Divider() }
            }
            Text("お手本音声は iOS の音声合成です。ネイティブの録音ではありません。")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .card()
    }
}

/// A large IPA symbol with its Latin label.
struct SoundGlyph: View {
    let sound: Sound
    var isMuted = false

    var body: some View {
        VStack(spacing: 2) {
            Text(sound.symbol)
                .font(.system(size: 56, weight: .semibold, design: .serif))
                .foregroundStyle(isMuted ? Color.secondary.opacity(0.5) : Color.brand)
            Text(sound.label).font(.caption).foregroundStyle(.secondary)
        }
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("\(sound.label)の音、\(sound.ipa)")
    }
}
