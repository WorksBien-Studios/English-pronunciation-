import Foundation

enum DailyAllowance {
    /// Free tier: ten valid scored recordings per local calendar day.
    static let freeLimit = 10

    static func dayKey(_ date: Date, calendar: Calendar = .current) -> String {
        let parts = calendar.dateComponents([.year, .month, .day], from: date)
        return String(format: "%04d-%02d-%02d", parts.year ?? 0, parts.month ?? 0, parts.day ?? 0)
    }

    /// `nil` means unlimited (Pro).
    static func remaining(used: Int, isPro: Bool) -> Int? {
        isPro ? nil : max(0, freeLimit - used)
    }

    /// Silence, clipping, interruptions and internal failures never consume an attempt.
    static func consumesAttempt(_ decision: EngineDecision) -> Bool {
        if case .result = decision { return true }
        return false
    }
}

enum StreakCalculator {
    /// Consecutive practice days ending today (or yesterday, if today has no practice yet).
    static func streak(days: Set<String>, today: Date, calendar: Calendar = .current) -> Int {
        var cursor = today
        if !days.contains(DailyAllowance.dayKey(cursor, calendar: calendar)) {
            guard let yesterday = calendar.date(byAdding: .day, value: -1, to: cursor) else { return 0 }
            cursor = yesterday
        }
        var count = 0
        while days.contains(DailyAllowance.dayKey(cursor, calendar: calendar)) {
            count += 1
            guard let previous = calendar.date(byAdding: .day, value: -1, to: cursor) else { break }
            cursor = previous
        }
        return count
    }
}
