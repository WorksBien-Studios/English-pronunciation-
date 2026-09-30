import Foundation

/// Calendar used for the Japanese UI: Gregorian with Japanese weekday symbols.
extension Calendar {
    static let japanese: Calendar = {
        var calendar = Calendar(identifier: .gregorian)
        calendar.locale = Locale(identifier: "ja_JP")
        calendar.timeZone = .autoupdatingCurrent
        return calendar
    }()
}

/// The last seven days ending today, for the week strip and the weekly chart.
enum WeekStrip {
    struct Day: Identifiable, Equatable {
        let id: String
        let date: Date
        /// 日 / 月 / 火 …
        let weekday: String
        let practiced: Bool
        let isToday: Bool
    }

    static func days(practiceDays: Set<String>, today: Date, calendar: Calendar = .japanese) -> [Day] {
        let symbols = calendar.veryShortStandaloneWeekdaySymbols
        let todayKey = DailyAllowance.dayKey(today, calendar: calendar)
        return (0..<7).compactMap { offset -> Day? in
            guard let date = calendar.date(byAdding: .day, value: offset - 6, to: today) else { return nil }
            let key = DailyAllowance.dayKey(date, calendar: calendar)
            let weekday = calendar.component(.weekday, from: date)
            return Day(
                id: key,
                date: date,
                weekday: symbols[(weekday - 1) % symbols.count],
                practiced: practiceDays.contains(key),
                isToday: key == todayKey
            )
        }
    }
}

/// What the learner can do with a stage right now.
enum StageAvailability: Equatable {
    case cleared(Int)
    case current
    /// The previous stage has not been cleared yet.
    case locked
    /// Needs Pro.
    case proOnly

    var isOpen: Bool {
        switch self {
        case .cleared, .current: true
        case .locked, .proOnly: false
        }
    }

    static func of(_ stage: Stage, in stages: [Stage], stars: (Stage) -> Int, isPro: Bool) -> StageAvailability {
        let earned = stars(stage)
        if earned > 0 { return .cleared(earned) }
        if !stage.isFree && !isPro { return .proOnly }
        if let previous = stages.first(where: { $0.number == stage.number - 1 }), stars(previous) == 0 {
            return .locked
        }
        return .current
    }
}
