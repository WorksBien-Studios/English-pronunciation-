import Foundation
import SwiftData

@Model
final class StageProgress {
    @Attribute(.unique) var stageID: String
    var stars: Int
    var updatedAt: Date

    init(stageID: String, stars: Int, updatedAt: Date = .now) {
        self.stageID = stageID
        self.stars = stars
        self.updatedAt = updatedAt
    }
}

/// One row per local calendar day with at least one valid analysis.
@Model
final class DailyUsage {
    @Attribute(.unique) var day: String
    var count: Int

    init(day: String, count: Int = 0) {
        self.day = day
        self.count = count
    }
}

/// "target>substitution" observation counts, e.g. "r>l". Shown only after
/// `EngineThresholds.minObservationsToShow` observations.
@Model
final class ErrorPatternCount {
    @Attribute(.unique) var pattern: String
    var count: Int

    init(pattern: String, count: Int = 0) {
        self.pattern = pattern
        self.count = count
    }
}

enum ErrorHistory {
    static func key(target: Sound, substitution: Sound) -> String {
        "\(target.rawValue)>\(substitution.rawValue)"
    }

    static func parse(_ pattern: String) -> (target: Sound, substitution: Sound)? {
        let parts = pattern.split(separator: ">").map(String.init)
        guard parts.count == 2, let target = Sound(rawValue: parts[0]),
              let substitution = Sound(rawValue: parts[1]) else { return nil }
        return (target, substitution)
    }

    @MainActor
    static func record(target: Sound, substitution: Sound, in context: ModelContext) {
        let patternKey = key(target: target, substitution: substitution)
        let descriptor = FetchDescriptor<ErrorPatternCount>(
            predicate: #Predicate { $0.pattern == patternKey }
        )
        if let existing = (try? context.fetch(descriptor))?.first {
            existing.count += 1
        } else {
            context.insert(ErrorPatternCount(pattern: patternKey, count: 1))
        }
    }
}

enum AppPersistence {
    static var schema: Schema {
        Schema([StageProgress.self, DailyUsage.self, ErrorPatternCount.self])
    }

    /// Falls back to an in-memory store if the on-disk store cannot be opened,
    /// so the app still launches (progress would then not persist).
    @MainActor
    static func makeContainer(inMemory: Bool = false) -> ModelContainer {
        do {
            return try ModelContainer(
                for: schema,
                configurations: ModelConfiguration(isStoredInMemoryOnly: inMemory)
            )
        } catch {
            // swiftlint:disable:next force_try
            return try! ModelContainer(
                for: schema,
                configurations: ModelConfiguration(isStoredInMemoryOnly: true)
            )
        }
    }
}
