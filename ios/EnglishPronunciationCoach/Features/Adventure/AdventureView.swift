import SwiftData
import SwiftUI

/// 冒険 tab: today's practice summary and the stage list.
/// One column on iPhone and narrow iPad windows; on wide windows the list sits beside the selected stage.
struct AdventureView: View {
    @Environment(EntitlementStore.self) private var entitlements
    @Query private var progress: [StageProgress]
    @Query private var usage: [DailyUsage]

    @State private var pushedStage: Stage?
    @State private var selectedStageID: String?
    @State private var showsPaywall = false

    private let catalog = StageCatalog.shared

    private var practiceDays: Set<String> {
        Set(usage.filter { $0.count > 0 }.map { $0.day })
    }

    private var usedToday: Int {
        usage.first { $0.day == DailyAllowance.dayKey(.now) }?.count ?? 0
    }

    private func stars(for stage: Stage) -> Int {
        progress.first { $0.stageID == stage.id }?.stars ?? 0
    }

    private func availability(of stage: Stage) -> StageAvailability {
        StageAvailability.of(stage, in: catalog.stages, stars: stars(for:), isPro: entitlements.isPro)
    }

    private var selectedStage: Stage? {
        if let id = selectedStageID, let stage = catalog.stages.first(where: { $0.id == id }) {
            return stage
        }
        return catalog.stages.first { availability(of: $0).isOpen } ?? catalog.stages.first
    }

    private func select(_ stage: Stage, wide: Bool) {
        switch availability(of: stage) {
        case .proOnly:
            showsPaywall = true
        case .locked:
            break
        case .cleared, .current:
            if wide {
                selectedStageID = stage.id
            } else {
                pushedStage = stage
            }
        }
    }

    var body: some View {
        Group {
            if catalog.stages.isEmpty {
                ContentUnavailableView(
                    "ステージを読み込めません",
                    systemImage: "exclamationmark.triangle",
                    description: Text("アプリを再起動してください。")
                )
            } else {
                WidthAdaptive {
                    HStack(spacing: 0) {
                        stageList(wide: true)
                            .frame(width: Layout.sidebarWidth)
                        Divider()
                        if let stage = selectedStage {
                            StageDetailView(stage: stage)
                                .id(stage.id)
                        }
                    }
                } compact: {
                    stageList(wide: false)
                }
            }
        }
        .navigationTitle("冒険")
        .navigationDestination(item: $pushedStage) { stage in
            StageDetailView(stage: stage)
        }
        .sheet(isPresented: $showsPaywall) { PaywallView() }
    }

    private func stageList(wide: Bool) -> some View {
        List {
            Section {
                TodayCard(
                    remaining: DailyAllowance.remaining(used: usedToday, isPro: entitlements.isPro),
                    usedToday: usedToday,
                    practiceDays: practiceDays
                )
            }
            Section("ステージ") {
                ForEach(catalog.stages) { stage in
                    let state = availability(of: stage)
                    Button {
                        select(stage, wide: wide)
                    } label: {
                        StageRow(stage: stage, state: state, showsChevron: !wide)
                    }
                    .buttonStyle(.plain)
                    .listRowBackground(
                        wide && stage.id == selectedStage?.id ? Color.brand.opacity(0.12) : Color(.secondarySystemGroupedBackground)
                    )
                    .accessibilityLabel(accessibilityText(stage, state))
                }
            }
        }
        .listStyle(.insetGrouped)
        .frame(maxWidth: wide ? nil : Layout.readableWidth + 40)
        .frame(maxWidth: .infinity)
    }

    private func accessibilityText(_ stage: Stage, _ state: StageAvailability) -> String {
        let base = "ステージ\(stage.number) \(stage.title)"
        switch state {
        case .cleared(let earned): return "\(base)、クリア済み、星\(earned)つ"
        case .current: return "\(base)、挑戦できます"
        case .locked: return "\(base)、前のステージをクリアすると開きます"
        case .proOnly: return "\(base)、Proで利用できます"
        }
    }
}

private struct StageRow: View {
    let stage: Stage
    let state: StageAvailability
    let showsChevron: Bool

    var body: some View {
        HStack(spacing: 12) {
            badge
            VStack(alignment: .leading, spacing: 2) {
                Text(stage.title).font(.headline)
                Text(stage.summary)
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                    .fixedSize(horizontal: false, vertical: true)
            }
            Spacer(minLength: 8)
            switch state {
            case .cleared(let earned): StarsView(count: earned)
            case .proOnly: ProBadge()
            case .current, .locked: EmptyView()
            }
            if showsChevron {
                Image(systemName: "chevron.right")
                    .font(.footnote.weight(.semibold))
                    .foregroundStyle(.tertiary)
            }
        }
        .frame(minHeight: 44)
        .contentShape(Rectangle())
    }

    private var badge: some View {
        Group {
            switch state {
            case .cleared, .current:
                Text("\(stage.number)").font(.headline).foregroundStyle(.white)
            case .locked, .proOnly:
                Image(systemName: "lock.fill").font(.subheadline).foregroundStyle(.secondary)
            }
        }
        .frame(width: 36, height: 36)
        .background(
            state.isOpen ? Color.brand : Color(.tertiarySystemFill),
            in: RoundedRectangle(cornerRadius: 9, style: .continuous)
        )
    }
}

/// Streak, remaining free analyses and the last seven days.
struct TodayCard: View {
    /// `nil` = unlimited (Pro).
    let remaining: Int?
    let usedToday: Int
    let practiceDays: Set<String>

    private var streak: Int {
        StreakCalculator.streak(days: practiceDays, today: .now, calendar: .japanese)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            HStack(spacing: 16) {
                allowanceGauge
                VStack(alignment: .leading, spacing: 4) {
                    Label {
                        Text(streak > 0 ? "\(streak)日連続で練習中" : "今日から始めましょう")
                    } icon: {
                        Image(systemName: "flame.fill").foregroundStyle(.orange)
                    }
                    .font(.headline)
                    Text(usedToday > 0 ? "今日は\(usedToday)回、分析しました。" : "今日はまだ練習していません。")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            WeekStripView(days: WeekStrip.days(practiceDays: practiceDays, today: .now))
        }
        .padding(.vertical, 4)
    }

    @ViewBuilder
    private var allowanceGauge: some View {
        if let remaining {
            Gauge(value: Double(remaining), in: 0...Double(DailyAllowance.freeLimit)) {
                Text("マイクパワー")
            } currentValueLabel: {
                Text("\(remaining)")
            }
            .gaugeStyle(.accessoryCircularCapacity)
            .tint(Color.brand)
            .scaleEffect(1.25)
            .frame(width: 68, height: 68)
            .accessibilityLabel("マイクパワー")
            .accessibilityValue("無料の分析 残り\(remaining)回")
        } else {
            Image(systemName: "infinity")
                .font(.title2.weight(.semibold))
                .foregroundStyle(Color.brand)
                .frame(width: 68, height: 68)
                .accessibilityLabel("Proのため、分析の回数に制限はありません")
        }
    }
}

struct WeekStripView: View {
    let days: [WeekStrip.Day]

    var body: some View {
        HStack {
            ForEach(days) { day in
                VStack(spacing: 6) {
                    ZStack {
                        Circle().fill(day.practiced ? Color.brand : Color(.tertiarySystemFill))
                        if day.practiced {
                            Image(systemName: "checkmark").font(.caption.bold()).foregroundStyle(.white)
                        }
                    }
                    .frame(width: 30, height: 30)
                    .overlay {
                        if day.isToday {
                            Circle().stroke(Color.brand, lineWidth: 2).padding(-4)
                        }
                    }
                    Text(day.weekday).font(.caption2).foregroundStyle(.secondary)
                }
                .frame(maxWidth: .infinity)
                .accessibilityElement(children: .ignore)
                .accessibilityLabel("\(day.weekday)曜日\(day.isToday ? "、今日" : "")、\(day.practiced ? "練習済み" : "練習なし")")
            }
        }
    }
}
