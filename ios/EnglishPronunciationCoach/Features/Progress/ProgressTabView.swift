import Charts
import SwiftData
import SwiftUI

/// 進捗 tab: streak, the week's analyses, and sounds that keep coming up as difficult.
struct ProgressTabView: View {
    @Query private var usage: [DailyUsage]
    @Query private var patterns: [ErrorPatternCount]

    private var practiceDays: Set<String> {
        Set(usage.filter { $0.count > 0 }.map { $0.day })
    }

    private var streak: Int {
        StreakCalculator.streak(days: practiceDays, today: .now, calendar: .japanese)
    }

    private var week: [WeekStrip.Day] {
        WeekStrip.days(practiceDays: practiceDays, today: .now)
    }

    private func count(on day: WeekStrip.Day) -> Int {
        usage.first { $0.day == day.id }?.count ?? 0
    }

    private var weekTotal: Int {
        week.reduce(0) { $0 + count(on: $1) }
    }

    private var shownPatterns: [ErrorPatternCount] {
        patterns
            .filter { $0.count >= EngineThresholds.minObservationsToShow }
            .sorted { $0.count > $1.count }
    }

    var body: some View {
        WidthAdaptive {
            HStack(spacing: 0) {
                List {
                    streakSection
                    patternsSection
                }
                .listStyle(.insetGrouped)
                .frame(width: Layout.sidebarWidth)
                Divider()
                List { chartSection }
                    .listStyle(.insetGrouped)
            }
        } compact: {
            List {
                streakSection
                chartSection
                patternsSection
            }
            .listStyle(.insetGrouped)
            .frame(maxWidth: Layout.readableWidth + 40)
            .frame(maxWidth: .infinity)
        }
        .navigationTitle("進捗")
    }

    private var streakSection: some View {
        Section("連続練習") {
            VStack(alignment: .leading, spacing: 14) {
                Label {
                    Text(streak > 0 ? "\(streak)日連続" : "まだ記録がありません")
                        .font(.title3.bold())
                } icon: {
                    Image(systemName: "flame.fill").foregroundStyle(.orange)
                }
                WeekStripView(days: week)
            }
            .padding(.vertical, 4)
        }
    }

    private var chartSection: some View {
        Section {
            VStack(alignment: .leading, spacing: 8) {
                HStack(alignment: .firstTextBaseline) {
                    Text("今週の分析回数").font(.headline)
                    Spacer()
                    Text("\(weekTotal)回").font(.body.monospacedDigit()).foregroundStyle(.secondary)
                }
                Chart(week) { day in
                    BarMark(
                        x: .value("日", day.date, unit: .day),
                        y: .value("回数", count(on: day))
                    )
                    .foregroundStyle(day.isToday ? Color.brand : Color.brand.opacity(0.35))
                    .cornerRadius(4)
                }
                .chartXAxis {
                    AxisMarks(values: .stride(by: .day)) { _ in
                        AxisValueLabel(format: .dateTime.weekday(.narrow), centered: true)
                    }
                }
                .chartYAxis {
                    AxisMarks(position: .leading)
                }
                .environment(\.locale, Locale(identifier: "ja_JP"))
                .frame(height: 180)
                .accessibilityLabel("今週の分析回数、合計\(weekTotal)回")
            }
            .padding(.vertical, 4)
        }
    }

    private var patternsSection: some View {
        Section {
            if shownPatterns.isEmpty {
                Text("同じ傾向が、確信度の高い判定で2回以上見つかると、ここに表示されます。")
                    .font(.footnote)
                    .foregroundStyle(.secondary)
            } else {
                ForEach(shownPatterns, id: \.pattern) { item in
                    if let parsed = ErrorHistory.parse(item.pattern) {
                        LabeledContent {
                            Text("\(item.count)回").monospacedDigit()
                        } label: {
                            Text("\(parsed.target.label) が \(parsed.substitution.label) に近く聞こえた")
                        }
                    }
                }
            }
        } header: {
            Text("苦手な音の傾向")
        }
    }
}
