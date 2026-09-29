import Foundation
import SwiftData
import SwiftUI
import UIKit

struct ClearSummary: Identifiable {
    let id = UUID()
    let stars: Int
    let outcome: PracticeOutcome
    /// Set the first time a stage is cleared: the new creature the learner collected.
    let newSound: Sound?
}

/// Full-screen recording challenge for one stage.
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
        ZStack {
            Palette.paleSky.ignoresSafeArea()
            if stage.words.isEmpty {
                ContentUnavailableView("単語がありません", systemImage: "text.badge.xmark")
            } else {
                content
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

    // MARK: Layout

    private var content: some View {
        VStack(spacing: 14) {
            topBar
            wordCard
            creatureArea
            Spacer(minLength: 0)
            statusArea
            controls
        }
        .padding(16)
    }

    private var topBar: some View {
        HStack(spacing: 12) {
            Button("やめる", action: onClose)
                .buttonStyle(.bordered)
                .tint(Palette.ink)
            ProgressBar(fraction: Double(model.index) / Double(max(1, stage.words.count)), fill: Palette.sun)
                .frame(height: 14)
            Text(model.progressText)
                .font(.game(14, relativeTo: .subheadline))
                .foregroundStyle(Palette.ink)
        }
    }

    private var wordCard: some View {
        VStack(spacing: 2) {
            Text("この単語を言ってみよう")
                .font(.game(13, relativeTo: .footnote))
                .foregroundStyle(Palette.secondaryText)
            Text(model.word.text)
                .font(.game(56, relativeTo: .largeTitle))
                .minimumScaleFactor(0.6)
            Text(model.word.ipa)
                .font(.game(18, relativeTo: .title3))
                .foregroundStyle(Palette.secondaryText)
            HStack {
                Button { model.playModel(slow: false) } label: {
                    Label("お手本", systemImage: "speaker.wave.2.fill")
                }
                Button { model.playModel(slow: true) } label: {
                    Label("ゆっくり", systemImage: "tortoise.fill")
                }
            }
            .buttonStyle(.bordered)
            .tint(Palette.indigo)
            .font(.game(13, relativeTo: .footnote))
            .padding(.top, 4)
        }
        .foregroundStyle(Palette.ink)
        .padding(14)
        .frame(maxWidth: .infinity)
        .gameCard(radius: 26)
    }

    private var creatureArea: some View {
        VStack(spacing: 6) {
            if let cue = stage.cues.first {
                Text("\(cue.part)：\(cue.instruction)")
                    .font(.game(15, relativeTo: .subheadline))
                    .foregroundStyle(Palette.ink)
                    .multilineTextAlignment(.center)
                    .padding(.horizontal, 16)
                    .padding(.vertical, 8)
                    .background(Capsule().fill(Color.white))
                    .overlay(Capsule().stroke(Palette.ink, lineWidth: 3))
            }
            CreatureView(
                sound: model.word.target,
                mouth: (model.phase == .ready || model.phase == .recording) ? .show : .rest
            )
            .frame(maxWidth: 200, maxHeight: 200)
        }
    }

    @ViewBuilder
    private var statusArea: some View {
        switch model.phase {
        case .ready:
            permissionMessage
        case .recording:
            VStack(spacing: 6) {
                HStack(spacing: 8) {
                    Circle().fill(Palette.coral).frame(width: 11, height: 11)
                    Text("録音中").font(.game(15, relativeTo: .subheadline)).foregroundStyle(Palette.coralEdge)
                }
                waveform
            }
        case .analyzing:
            HStack(spacing: 8) {
                ProgressView()
                Text("分析中…").font(.game(15, relativeTo: .subheadline)).foregroundStyle(Palette.ink)
            }
        case .feedback(let decision):
            feedbackCard(decision)
        case .finished:
            EmptyView()
        }
    }

    @ViewBuilder
    private var permissionMessage: some View {
        switch model.recorder.state {
        case .denied:
            VStack(spacing: 8) {
                Text("マイクの使用が許可されていません。設定で許可すると録音できます。")
                    .font(.game(13, relativeTo: .footnote))
                    .multilineTextAlignment(.center)
                    .foregroundStyle(Palette.ink)
                Button("設定を開く") {
                    if let url = URL(string: UIApplication.openSettingsURLString) {
                        UIApplication.shared.open(url)
                    }
                }
                .buttonStyle(.bordered)
            }
        case .failed:
            Text("録音を始められませんでした。もう一度お試しください。")
                .font(.game(13, relativeTo: .footnote))
                .foregroundStyle(Palette.ink)
        default:
            if let remaining {
                Text("今日の無料チャレンジ 残り\(remaining)回")
                    .font(.game(12, relativeTo: .caption))
                    .foregroundStyle(Palette.secondaryText)
            }
        }
    }

    private var waveform: some View {
        HStack(spacing: 5) {
            ForEach(0..<16, id: \.self) { index in
                let base = 0.25 + 0.75 * abs(sin(Double(index) * 0.9))
                let height = 8 + CGFloat(base) * CGFloat(model.recorder.level) * 56
                Capsule()
                    .fill(index % 3 == 0 ? Palette.indigo : Palette.teal)
                    .frame(width: 7, height: max(8, height))
            }
        }
        .frame(height: 64)
        .animation(.easeOut(duration: 0.1), value: model.recorder.level)
        .accessibilityHidden(true)
    }

    // MARK: Feedback

    @ViewBuilder
    private func feedbackCard(_ decision: EngineDecision) -> some View {
        switch decision {
        case .retry(let reason):
            VStack(spacing: 6) {
                Text(retryMessage(for: reason))
                    .font(.game(14, relativeTo: .subheadline))
                    .multilineTextAlignment(.center)
                Text("この録音は無料回数に数えません。")
                    .font(.game(11, relativeTo: .caption2))
                    .foregroundStyle(Palette.secondaryText)
            }
            .foregroundStyle(Palette.ink)
            .padding(14)
            .frame(maxWidth: .infinity)
            .gameCard()
        case .result(let result):
            VStack(alignment: .leading, spacing: 8) {
                Text(result.targetSoundProduced ? "いいね！" : "おしい！あと少し")
                    .font(.game(17, relativeTo: .headline))
                    .foregroundStyle(Palette.indigo)
                resultRow(title: "相手に伝わりそうか", good: result.intelligible, goodText: "伝わりそう", otherText: "もう少し")
                resultRow(title: "目標の音 \(model.word.target.ipa)", good: result.targetSoundProduced, goodText: "出せた", otherText: "もう少し")
                if !result.targetSoundProduced {
                    if let substitution = result.likelySubstitution {
                        Text("\(substitution.ipa) に近く聞こえた可能性があります。分析には限りがあり、断定はできません。")
                            .font(.game(12, relativeTo: .caption))
                            .foregroundStyle(Palette.secondaryText)
                    }
                    if let cue = stage.cues.first {
                        Text("次の1回で試すこと：\(cue.instruction)")
                            .font(.game(14, relativeTo: .subheadline))
                    }
                }
            }
            .foregroundStyle(Palette.ink)
            .padding(14)
            .frame(maxWidth: .infinity, alignment: .leading)
            .gameCard()
        }
    }

    private func resultRow(title: String, good: Bool, goodText: String, otherText: String) -> some View {
        HStack {
            Text(title).font(.game(14, relativeTo: .subheadline))
            Spacer()
            Label(good ? goodText : otherText, systemImage: good ? "checkmark.circle.fill" : "minus.circle.fill")
                .font(.game(14, relativeTo: .subheadline))
                .foregroundStyle(good ? Palette.teal : Color(hex: 0xC77700))
        }
    }

    private func retryMessage(for reason: RetryReason) -> String {
        switch reason {
        case .silence: "声がうまく聞き取れませんでした。静かな場所でもう一度。"
        case .clipped: "音が大きすぎたようです。少し離れてもう一度。"
        case .tooShort: "録音が短すぎました。最後まで言ってみましょう。"
        case .lowConfidence: "はっきり判定できませんでした。もう一度言ってみましょう。"
        case .engineUnavailable: "この端末では判定の準備がまだできていません。"
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
                Label("録音する", systemImage: "mic.fill")
            }
            .buttonStyle(.chunky)
        case .recording:
            Button {
                Task { await model.stopAndAnalyze(onValid: handleValid) }
            } label: {
                Label("録音を終える", systemImage: "stop.fill")
            }
            .buttonStyle(ChunkyButtonStyle(tone: .danger))
        case .analyzing, .finished:
            EmptyView()
        case .feedback(let decision):
            HStack(spacing: 12) {
                Button("もう一度") { model.retry() }
                    .buttonStyle(ChunkyButtonStyle(tone: .secondary))
                if case .result = decision {
                    Button(model.isLastWord ? "結果を見る" : "つぎへ") { model.advance() }
                        .buttonStyle(.chunky)
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
