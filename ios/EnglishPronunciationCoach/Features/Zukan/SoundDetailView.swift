import SwiftData
import SwiftUI

/// Detail for one 図鑑 entry: example words, the confusion note, and the way into the stage.
@MainActor
struct SoundDetailView: View {
    let sound: Sound
    let stage: Stage?
    let isCollected: Bool

    @Environment(EntitlementStore.self) private var entitlements
    @Query private var progress: [StageProgress]
    @State private var player = ModelAudioPlayer()
    @State private var challengeStage: Stage?
    @State private var showsPaywall = false

    private var examples: [PracticeWord] {
        stage?.words.filter { $0.target == sound } ?? []
    }

    /// The stage cues describe the stage's main sound only.
    private var cues: [ArticulationCue] {
        guard let stage, stage.primarySound == sound else { return [] }
        return stage.cues
    }

    private var practiceState: StageAvailability? {
        guard let stage else { return nil }
        let stages = StageCatalog.shared.stages
        return StageAvailability.of(
            stage,
            in: stages,
            stars: { target in progress.first { $0.stageID == target.id }?.stars ?? 0 },
            isPro: entitlements.isPro
        )
    }

    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                header
                if !examples.isEmpty { examplesCard }
                if !cues.isEmpty { CueList(cues: cues) }
                practiceButton
            }
            .padding(16)
            .readableColumn()
        }
        .background(Color(.systemGroupedBackground))
        .navigationTitle("\(sound.label) \(sound.ipa)")
        .navigationBarTitleDisplayMode(.inline)
        .fullScreenCover(item: $challengeStage) { current in
            ChallengeView(stage: current) { challengeStage = nil }
        }
        .sheet(isPresented: $showsPaywall) { PaywallView() }
    }

    private var header: some View {
        VStack(spacing: 4) {
            SoundGlyph(sound: sound, isMuted: !isCollected)
            Text(sound.japaneseNote)
                .font(.subheadline)
                .foregroundStyle(.secondary)
        }
        .padding(.vertical, 8)
        .frame(maxWidth: .infinity)
        .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
    }

    private var examplesCard: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("例").font(.headline)
            ForEach(examples) { word in
                WordRow(word: word, player: player)
                if word.id != examples.last?.id { Divider() }
            }
        }
        .card()
    }

    @ViewBuilder
    private var practiceButton: some View {
        if let stage, let practiceState {
            switch practiceState {
            case .cleared, .current:
                Button {
                    challengeStage = stage
                } label: {
                    Label("\(stage.title)で練習する", systemImage: "mic.fill").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.large)
            case .proOnly:
                Button {
                    showsPaywall = true
                } label: {
                    Label("Proで練習する", systemImage: "lock.fill").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .controlSize(.large)
            case .locked:
                Text("前のステージをクリアすると練習できます。")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            }
        }
    }
}
