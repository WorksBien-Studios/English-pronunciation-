import SwiftUI

/// One stage: the sounds, how to make them, and the challenge entry point.
@MainActor
struct StageDetailView: View {
    let stage: Stage

    @State private var challengeStage: Stage?
    @State private var player = ModelAudioPlayer()

    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                header
                creaturesCard
                if stage.showsRLDiagram {
                    MouthDiagramView()
                        .padding(.horizontal, 2)
                }
                cuesCard
                wordsCard
            }
            .padding(16)
        }
        .background(SkyBackground())
        .safeAreaInset(edge: .bottom) {
            Button {
                challengeStage = stage
            } label: {
                Label("チャレンジ！", systemImage: "mic.fill")
            }
            .buttonStyle(.chunky)
            .padding(.horizontal, 16)
            .padding(.vertical, 10)
            .background(.ultraThinMaterial)
        }
        .navigationTitle("ステージ\(stage.number)")
        .navigationBarTitleDisplayMode(.inline)
        .fullScreenCover(item: $challengeStage) { current in
            ChallengeView(stage: current) { challengeStage = nil }
        }
    }

    private var header: some View {
        VStack(spacing: 6) {
            OutlinedText(text: "ステージ\(stage.number)・\(stage.title)", size: 22)
            Text(stage.summary)
                .font(.game(14, relativeTo: .subheadline))
                .foregroundStyle(Palette.ink)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity)
    }

    private var creaturesCard: some View {
        VStack(spacing: 8) {
            HStack(spacing: 14) {
                ForEach(Array(stage.sounds.enumerated()), id: \.element.id) { index, sound in
                    if index > 0 {
                        Image(systemName: "arrow.left.arrow.right")
                            .font(.title3.weight(.bold))
                            .foregroundStyle(Palette.ink)
                            .accessibilityHidden(true)
                    }
                    CreatureView(sound: sound)
                        .frame(maxWidth: 104, maxHeight: 104)
                }
            }
            Text(pairCaption)
                .font(.game(14, relativeTo: .subheadline))
                .foregroundStyle(Palette.ink)
                .multilineTextAlignment(.center)
        }
        .padding(14)
        .frame(maxWidth: .infinity)
        .gameCard()
    }

    private var pairCaption: String {
        let names = stage.sounds.map { "「\($0.displayName)」" }
        return names.count > 1 ? names.joined(separator: "と") + "を聞き分けよう！" : (names.first ?? "") + "を練習しよう！"
    }

    private var cuesCard: some View {
        VStack(alignment: .leading, spacing: 0) {
            Text("口のポイント")
                .font(.game(15, relativeTo: .headline))
                .padding(.bottom, 8)
            ForEach(stage.cues) { cue in
                HStack(alignment: .firstTextBaseline, spacing: 14) {
                    Text(cue.part)
                        .font(.game(17, relativeTo: .headline))
                        .foregroundStyle(Palette.indigo)
                        .frame(width: 28, alignment: .leading)
                    Text(cue.instruction)
                        .font(.game(15, relativeTo: .body))
                        .foregroundStyle(Palette.ink)
                }
                .padding(.vertical, 6)
                if cue.id != stage.cues.last?.id { Divider() }
            }
        }
        .padding(14)
        .frame(maxWidth: .infinity, alignment: .leading)
        .gameCard()
    }

    private var wordsCard: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("練習する単語")
                .font(.game(15, relativeTo: .headline))
            ForEach(stage.words) { word in
                HStack(spacing: 10) {
                    VStack(alignment: .leading, spacing: 0) {
                        Text(word.text).font(.game(18, relativeTo: .title3))
                        Text(word.ipa).font(.game(13, relativeTo: .footnote)).foregroundStyle(Palette.secondaryText)
                    }
                    Spacer()
                    Button {
                        player.speak(word.text, slow: false)
                    } label: {
                        Label("お手本", systemImage: "speaker.wave.2.fill").font(.game(13, relativeTo: .footnote))
                    }
                    Button {
                        player.speak(word.text, slow: true)
                    } label: {
                        Label("ゆっくり", systemImage: "tortoise.fill").font(.game(13, relativeTo: .footnote))
                    }
                }
                .buttonStyle(.bordered)
                .tint(Palette.indigo)
            }
            Text("お手本音声は iOS 標準の音声合成です。")
                .font(.game(11, relativeTo: .caption2))
                .foregroundStyle(Palette.secondaryText)
        }
        .foregroundStyle(Palette.ink)
        .padding(14)
        .frame(maxWidth: .infinity, alignment: .leading)
        .gameCard()
    }
}
