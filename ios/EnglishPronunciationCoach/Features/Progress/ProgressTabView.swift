import SwiftData
import SwiftUI

/// 進捗 tab: practice streak, a stamp calendar for the month, and recurring weak sounds.
struct ProgressTabView: View {
    @Query private var usage: [DailyUsage]
    @Query private var patterns: [ErrorPatternCount]

    private struct DayCell: Identifiable {
        let id: Int
        let day: Int?
        let key: String?
    }

    private var practiceDays: Set<String> {
        Set(usage.filter { $0.count > 0 }.map { $0.day })
    }

    private var streak: Int {
        StreakCalculator.streak(days: practiceDays, today: .now)
    }

    private var shownPatterns: [ErrorPatternCount] {
        patterns
            .filter { $0.count >= EngineThresholds.minObservationsToShow }
            .sorted { $0.count > $1.count }
    }

    private var weekdayHeaders: [String] {
        let calendar = Calendar.current
        let symbols = calendar.veryShortStandaloneWeekdaySymbols
        let first = max(0, min(symbols.count - 1, calendar.firstWeekday - 1))
        return Array(symbols[first...]) + Array(symbols[..<first])
    }

    private func monthCells() -> [DayCell] {
        let calendar = Calendar.current
        guard let monthStart = calendar.date(from: calendar.dateComponents([.year, .month], from: .now)),
              let range = calendar.range(of: .day, in: .month, for: monthStart) else { return [] }
        let weekday = calendar.component(.weekday, from: monthStart)
        let lead = (weekday - calendar.firstWeekday + 7) % 7
        var cells: [DayCell] = (0..<lead).map { DayCell(id: -($0 + 1), day: nil, key: nil) }
        for day in range {
            let date = calendar.date(byAdding: .day, value: day - 1, to: monthStart) ?? monthStart
            cells.append(DayCell(id: day, day: day, key: DailyAllowance.dayKey(date, calendar: calendar)))
        }
        return cells
    }

    var body: some View {
        ScrollView {
            VStack(spacing: 16) {
                streakCard
                calendarCard
                patternsCard
            }
            .padding(16)
            .frame(maxWidth: 720)
            .frame(maxWidth: .infinity)
        }
        .background(SkyBackground())
        .navigationTitle("進捗")
    }

    private var streakCard: some View {
        HStack(spacing: 14) {
            CoachView(mood: .happy).frame(width: 72, height: 72)
            VStack(alignment: .leading, spacing: 2) {
                Text("連続練習").font(.game(13, relativeTo: .footnote)).foregroundStyle(Palette.secondaryText)
                Text("\(streak)日").font(.game(36, relativeTo: .largeTitle))
                Text("今月 \(monthPracticeCount)日 練習しました").font(.game(13, relativeTo: .footnote))
            }
            .foregroundStyle(Palette.ink)
            Spacer(minLength: 0)
        }
        .padding(14)
        .gameCard(radius: 24)
        .accessibilityElement(children: .combine)
    }

    private var monthPracticeCount: Int {
        let days = practiceDays
        return monthCells().filter { cell in cell.key.map { days.contains($0) } ?? false }.count
    }

    private var calendarCard: some View {
        let cells = monthCells()
        let today = DailyAllowance.dayKey(.now)
        let days = practiceDays
        return VStack(alignment: .leading, spacing: 8) {
            Text("今月のスタンプ").font(.game(15, relativeTo: .headline))
            LazyVGrid(columns: Array(repeating: GridItem(.flexible(), spacing: 4), count: 7), spacing: 4) {
                ForEach(Array(weekdayHeaders.enumerated()), id: \.offset) { _, symbol in
                    Text(symbol).font(.game(12, relativeTo: .caption)).foregroundStyle(Palette.secondaryText)
                }
                ForEach(cells) { cell in
                    dayView(cell, today: today, days: days)
                        .frame(height: 42)
                }
            }
        }
        .foregroundStyle(Palette.ink)
        .padding(14)
        .gameCard(radius: 24)
    }

    @ViewBuilder
    private func dayView(_ cell: DayCell, today: String, days: Set<String>) -> some View {
        if let day = cell.day, let key = cell.key {
            if days.contains(key) {
                ZStack {
                    StampView().frame(width: 38, height: 38)
                    Text("\(day)").font(.game(11, relativeTo: .caption2)).foregroundStyle(Color(hex: 0x9B2C2C))
                }
                .accessibilityLabel("\(day)日 練習済み")
            } else if key == today {
                Text("\(day)")
                    .font(.game(13, relativeTo: .footnote))
                    .frame(width: 36, height: 36)
                    .background(Circle().fill(Palette.paleSky))
                    .overlay(Circle().stroke(Palette.indigo, lineWidth: 2.5))
                    .accessibilityLabel("\(day)日 今日")
            } else {
                Text("\(day)").font(.game(13, relativeTo: .footnote)).foregroundStyle(Palette.secondaryText)
            }
        } else {
            Color.clear
        }
    }

    private var patternsCard: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("繰り返し見つかった苦手な音").font(.game(15, relativeTo: .headline))
            if shownPatterns.isEmpty {
                Text("同じパターンが確信度の高い形で2回以上確認された音だけを記録します。まだ記録はありません。")
                    .font(.game(13, relativeTo: .footnote))
                    .foregroundStyle(Palette.secondaryText)
            } else {
                ForEach(shownPatterns, id: \.pattern) { item in
                    if let parsed = ErrorHistory.parse(item.pattern) {
                        HStack {
                            Text("\(parsed.target.ipa) → \(parsed.substitution.ipa)")
                                .font(.game(17, relativeTo: .headline))
                            Spacer()
                            Text("\(item.count)回").font(.game(14, relativeTo: .subheadline)).foregroundStyle(Palette.secondaryText)
                        }
                        .accessibilityElement(children: .combine)
                    }
                }
            }
        }
        .foregroundStyle(Palette.ink)
        .padding(14)
        .frame(maxWidth: .infinity, alignment: .leading)
        .gameCard(radius: 24)
    }
}

/// Red 花丸-style stamp.
struct StampView: View {
    var body: some View {
        Canvas { context, size in
            var ctx = context
            ctx.translateBy(x: size.width / 2, y: size.height / 2)
            let scale = min(size.width, size.height) / 40
            ctx.scaleBy(x: scale, y: scale)
            let red = Color(hex: 0xD64545)
            for index in 0..<8 {
                var petal = ctx
                petal.rotate(by: .degrees(Double(index) * 45))
                petal.stroke(Path(ellipseIn: CGRect(x: -6, y: -19, width: 12, height: 16)), with: .color(red), lineWidth: 1.8)
            }
            ctx.stroke(Path(ellipseIn: CGRect(x: -7, y: -7, width: 14, height: 14)), with: .color(red), lineWidth: 1.8)
        }
        .accessibilityHidden(true)
    }
}
