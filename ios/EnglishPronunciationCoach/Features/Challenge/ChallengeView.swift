import Foundation
import SwiftData
import SwiftUI
import UIKit

struct ClearSummary: Identifiable {
    let id = UUID()
    let stars: Int
    let outcome: PracticeOutcome
    /// Set the first time a stage is cleared: the sound added to the 図鑑.
    let newSound: Sound?
}

/// Full-screen recording challenge for one stage: word → record → result → next.
/// On wide windows the word list sits beside the recording area.
@MainActor
struct ChallengeView: View {
    let stage: Stage
    let onClose: () -> Void

    @Environment(\.modelContext) private var modelContext
    @Environment(EntitlementStore.self) private var entitlements
    @Query private var usage: [DailyUsage]
    @Query private var progress: [StageProgress]

    @State private var model: ChallengeViewModel
    @State private var showsPaywall = false
    @State private var summary: ClearSummary?

    init(stage: Stage, onClose: @escaping () -> Void) {
        self.stage = stage
        self.onClose = onClose
        _model = State(initialValue: ChallengeViewModel(stage: stage, engine: EngineFactory.makeDefault()))
    }

    private var usedToday: Int {
        usage.first { $0.day == DailyAllowance.dayKey(.now) }?.count ?? 0
    }

    private var remaining: Int? {
        DailyAllowance.remaining(used: usedToday, isPro: entitlements.isPro)
    }

    var body: some View {
        NavigationStack {
            Group {
                if stage.words.isEmpty {
                    ContentUnavailableView("単語がありません", systemImage: "text.badge.xmark")
                } else {
                    WidthAdaptive {
                        HStack(spacing: 0) {
                            wordList.frame(width: 320)
                            Divider()
                            practiceArea
                        }
                    } compact: {
                        practiceArea
                    }
                }
            }
            .background(Color(.systemGroupedBackground))
            .navigationTitle("録音チャレンジ")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("やめる", action: onClose)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Text(model.progressText)
                        .font(.body.monospacedDigit())
                        .foregroundStyle(.secondary)
                        .accessibilityLabel("\(model.index + 1)語目、全\(stage.words.count)語")
                }
            }
        }
        .onChange(of: model.phase) { _, newPhase in
            if newPhase == .finished { finishStage() }
        }
        .fullScreenCover(item: $summary) { current in
            ClearView(stage: stage, summary: current) {
                summary = nil
                onClose()
            } onRetry: {
                summary = nil
                onClose()
            }
        }
        .sheet(isPresented: $showsPaywall) { PaywallView() }
    }

    // MARK: Word list (wide layout)

    private var wordList: some View {
        List {
            Section("\(stage.title)の単語") {
                ForEach(Array(stage.words.enumerated()), id: \.element.id) { index, word in
                    HStack {
                        VStack(alignment: .leading, spacing: 0) {
                            Text(word.text).font(.headline)
                            Text(word.ipa).font(.footnote).foregroundStyle(.secondary)
                        }
                        Spacer()
                        if index < model.index {
                            Image(systemName: "checkmark.circle.fill").foregroundStyle(.green)
                        }
                    }
                    .frame(minHeight: 44)
                    .listRowBackground(index == model.index ? Color.brand.opacity(0.12) : Color(.secondarySystemGroupedBackground))
                }
            }
        }
        .listStyle(.insetGrouped)
    }

    // MARK: Practice area

    private var practiceArea: some View {
        VStack(spacing: 16) {
            ScrollView {
                VStack(spacing: 16) {
                    wordHeader
                    statusArea
                }
                .padding(16)
                .readableColumn()
            }
            controls
                .padding(.horizontal, 16)
                .padding(.vertical, 10)
                .frame(maxWidth: Layout.readableWidth + 32)
                .frame(maxWidth: .infinity)
                .background(.bar)
        }
    }

    private var wordHeader: some View {
        VStack(spacing: 4) {
            Text("この単語を言ってみましょう")
                .font(.footnote)
                .foregroundStyle(.secondary)
            Text(model.word.text)
                .font(.system(size: 60, weight: .bold))
                .minimumScaleFactor(0.5)
                .lineLimit(1)
            Text(model.word.ipa)
                .font(.system(.title2, design: .serif))
                .foregroundStyle(.secondary)
            HStack {
                Button { model.playModel(slow: false) } label: {
                    Label("お手本", systemImage: "speaker.wave.2.fill")
                }
                Button { model.playModel(slow: true) } label: {
                    Label("ゆっくり", systemImage: "tortoise.fill")
                }
            }
            .buttonStyle(.bordered)
            .controlSize(.small)
            .padding(.top, 6)
        }
        .padding(.top, 8)
        .accessibilityElement(children: .contain)
    }

    @ViewBuilder
    private var statusArea: some View {
        switch model.phase {
        case .ready:
            VStack(spacing: 12) {
                if let cue = stage.cues.first {
                    Label("\(cue.part)：\(cue.instruction)", systemImage: "lightbulb")
                        .font(.subheadline)
                        .card()
                }
                permissionMessage
            }
        case .recording:
            VStack(spacing: 10) {
                Label("録音中", systemImage: "circle.fill")
                    .font(.subheadline.weight(.semibold))
                    .foregroundStyle(.red)
                LiveWaveform(level: model.recorder.level)
            }
            .card()
        case .analyzing:
            HStack(spacing: 8) {
                ProgressView()
                Text("分析しています…").font(.subheadline)
            }
            .padding(.top, 24)
        case .feedback(let decision):
            ResultCard(stage: stage, word: model.word, decision: decision)
        case .finished:
            EmptyView()
        }
    }

    @ViewBuilder
    private var permissionMessage: some View {
        switch model.recorder.state {
        case .denied:
            VStack(spacing: 8) {
                Text("マイクの使用が許可されていません。設定アプリで許可すると録音できます。")
                    .font(.footnote)
                    .multilineTextAlignment(.center)
                Button("設定を開く") {
                    if let url = URL(string: UIApplication.openSettingsURLString) {
                        UIApplication.shared.open(url)
                    }
                }
                .buttonStyle(.bordered)
            }
        case .failed:
            Text("録音を始められませんでした。もう一度お試しください。")
                .font(.footnote)
        default:
            VStack(spacing: 4) {
                if let remaining {
                    Text("今日の無料分析 残り\(remaining)回")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                Text("音声は端末の中だけで処理し、保存しません。")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
        }
    }

    // MARK: Controls

    @ViewBuilder
    private var controls: some View {
        switch model.phase {
        case .ready:
            Button {
                startTapped()
            } label: {
                Label("録音する", systemImage: "mic.fill").frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
        case .recording:
            Button {
                Task { await model.stopAndAnalyze(onValid: handleValid) }
            } label: {
                Label("録音を終える", systemImage: "stop.fill").frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.large)
            .tint(.red)
            .sensoryFeedback(.impact, trigger: model.phase == .recording)
        case .analyzing, .finished:
            Color.clear.frame(height: 0)
        case .feedback(let decision):
            HStack(spacing: 12) {
                Button {
                    model.retry()
                } label: {
                    Label("もう一度", systemImage: "arrow.counterclockwise").frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)
                .controlSize(.large)
                if case .result = decision {
                    Button {
                        model.advance()
                    } label: {
                        Text(model.isLastWord ? "結果を見る" : "つぎへ").frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .controlSize(.large)
                }
            }
        }
    }

    private func startTapped() {
        if let remaining, remaining == 0 {
            showsPaywall = true
            return
        }
        Task { await model.startRecording() }
    }

    // MARK: Persistence

    private func handleValid(_ result: PronunciationResult) {
        let key = DailyAllowance.dayKey(.now)
        if let today = usage.first(where: { $0.day == key }) {
            today.count += 1
        } else {
            modelContext.insert(DailyUsage(day: key, count: 1))
        }
        if result.confidence >= EngineThresholds.recordConfidence,
           !result.targetSoundProduced,
           let substitution = result.likelySubstitution {
            ErrorHistory.record(target: model.word.target, substitution: substitution, in: modelContext)
        }
    }

    private func finishStage() {
        let outcome = model.outcome
        let earned = StarRating.stars(for: outcome)
        let previous = progress.first { $0.stageID == stage.id }
        let wasCleared = (previous?.stars ?? 0) > 0
        if let previous {
            previous.stars = max(previous.stars, earned)
            previous.updatedAt = .now
        } else if earned > 0 {
            modelContext.insert(StageProgress(stageID: stage.id, stars: earned))
        }
        summary = ClearSummary(
            stars: earned,
            outcome: outcome,
            newSound: (!wasCleared && earned > 0) ? stage.primarySound : nil
        )
    }
}

/// The result of one recording. A retry never shows a definitive judgement.
struct ResultCard: View {
    let stage: Stage
    let word: PracticeWord
    let decision: EngineDecision

    var body: some View {
        switch decision {
        case .retry(let reason):
            VStack(spacing: 6) {
                Label(retryMessage(for: reason), systemImage: "arrow.counterclockwise.circle")
                    .font(.subheadline)
                Text("この録音は無料回数に数えません。")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            .card()
        case .result(let result):
            VStack(alignment: .leading, spacing: 12) {
                Text(result.targetSoundProduced ? "目標の音が出せました" : "おしい！あと少し")
                    .font(.title3.bold())
                VStack(spacing: 0) {
                    row("相手に伝わりそうか", good: result.intelligible, goodText: "伝わりそう", otherText: "もう少し")
                    Divider()
                    row("目標の音 \(word.target.ipa)", good: result.targetSoundProduced, goodText: "出せた", otherText: "もう少し")
                }
                if !result.targetSoundProduced {
                    if let substitution = result.likelySubstitution {
                        Text("\(substitution.ipa) に近く聞こえた可能性があります。分析には限りがあるため、断定はできません。")
                            .font(.footnote)
                            .foregroundStyle(.secondary)
                    }
                    if let cue = stage.cues.first {
                        Label("次の1回で試すこと：\(cue.instruction)", systemImage: "arrow.turn.down.right")
                            .font(.subheadline)
                    }
                }
            }
            .card()
        }
    }

    private func row(_ title: String, good: Bool, goodText: String, otherText: String) -> some View {
        HStack {
            Text(title).font(.subheadline)
            Spacer()
            Label(good ? goodText : otherText, systemImage: good ? "checkmark.circle.fill" : "minus.circle.fill")
                .font(.subheadline)
                .foregroundStyle(good ? Color.green : Color.orange)
        }
        .padding(.vertical, 10)
    }

    private func retryMessage(for reason: RetryReason) -> String {
        switch reason {
        case .silence: "声がうまく聞き取れませんでした。静かな場所で、もう一度お試しください。"
        case .clipped: "音が大きすぎたようです。マイクから少し離れて、もう一度お試しください。"
        case .tooShort: "録音が短すぎました。単語を最後まで言ってください。"
        case .lowConfidence: "はっきり判定できませんでした。もう一度お試しください。"
        case .engineUnavailable: "この端末では判定の準備がまだできていません。"
        }
    }
}
