import XCTest
@testable import EnglishPronunciationCoach

final class SoundTests: XCTestCase {
    func testSixUniqueSoundsWithGenericNames() {
        XCTAssertEqual(Sound.allCases.count, 6)
        XCTAssertEqual(Set(Sound.allCases.map { $0.displayName }).count, 6)
        XCTAssertEqual(Set(Sound.allCases.map { $0.ipa }).count, 6)
        XCTAssertEqual(Sound.r.displayName, "Rくん")
        XCTAssertEqual(Sound.th.ipa, "/θ/")
    }
}

final class StarRatingTests: XCTestCase {
    func testStarsSeparateUnderstandabilityTargetSoundAndStability() {
        XCTAssertEqual(StarRating.stars(for: PracticeOutcome(intelligible: false, targetSoundProduced: true, stable: true)), 0)
        XCTAssertEqual(StarRating.stars(for: PracticeOutcome(intelligible: true, targetSoundProduced: false, stable: false)), 1)
        XCTAssertEqual(StarRating.stars(for: PracticeOutcome(intelligible: true, targetSoundProduced: true, stable: false)), 2)
        XCTAssertEqual(StarRating.stars(for: PracticeOutcome(intelligible: true, targetSoundProduced: true, stable: true)), 3)
    }

    func testOutcomeFromResultsRequiresTwoTrailingSuccessesForStability() {
        func result(target: Bool) -> PronunciationResult {
            PronunciationResult(intelligible: true, targetSoundProduced: target, likelySubstitution: nil, confidence: 0.9)
        }
        XCTAssertEqual(OutcomeCalculator.outcome(from: []), PracticeOutcome(intelligible: false, targetSoundProduced: false, stable: false))
        XCTAssertEqual(
            OutcomeCalculator.outcome(from: [result(target: true)]),
            PracticeOutcome(intelligible: true, targetSoundProduced: true, stable: false)
        )
        XCTAssertEqual(
            OutcomeCalculator.outcome(from: [result(target: false), result(target: true), result(target: true)]),
            PracticeOutcome(intelligible: true, targetSoundProduced: true, stable: true)
        )
        XCTAssertFalse(OutcomeCalculator.outcome(from: [result(target: true), result(target: false)]).stable)
    }

    func testPlayerLevelProgression() {
        XCTAssertEqual(PlayerLevel.level(totalStars: 0), 1)
        XCTAssertEqual(PlayerLevel.level(totalStars: 3), 2)
        XCTAssertEqual(PlayerLevel.expToNextLevel(totalStars: 0), PlayerLevel.expPerLevel)
    }
}

final class DailyAllowanceTests: XCTestCase {
    func testFreeLimitAndProUnlimited() {
        XCTAssertEqual(DailyAllowance.remaining(used: 3, isPro: false), 7)
        XCTAssertEqual(DailyAllowance.remaining(used: 12, isPro: false), 0)
        XCTAssertNil(DailyAllowance.remaining(used: 99, isPro: true))
    }

    func testOnlyValidResultsConsumeAnAttempt() {
        let valid = PronunciationResult(intelligible: true, targetSoundProduced: true, likelySubstitution: nil, confidence: 0.9)
        XCTAssertTrue(DailyAllowance.consumesAttempt(.result(valid)))
        for reason in [RetryReason.silence, .clipped, .tooShort, .lowConfidence, .engineUnavailable] {
            XCTAssertFalse(DailyAllowance.consumesAttempt(.retry(reason)))
        }
    }

    func testDayKeyAndStreak() {
        var calendar = Calendar(identifier: .gregorian)
        calendar.timeZone = TimeZone(secondsFromGMT: 0)!
        let today = calendar.date(from: DateComponents(year: 2026, month: 9, day: 29))!
        XCTAssertEqual(DailyAllowance.dayKey(today, calendar: calendar), "2026-09-29")

        let days: Set<String> = ["2026-09-27", "2026-09-28", "2026-09-29", "2026-09-25"]
        XCTAssertEqual(StreakCalculator.streak(days: days, today: today, calendar: calendar), 3)
        // No practice yet today: the streak still counts up to yesterday.
        XCTAssertEqual(StreakCalculator.streak(days: ["2026-09-27", "2026-09-28"], today: today, calendar: calendar), 2)
        XCTAssertEqual(StreakCalculator.streak(days: ["2026-09-20"], today: today, calendar: calendar), 0)
    }
}

final class ErrorHistoryTests: XCTestCase {
    func testKeyRoundTrip() {
        let key = ErrorHistory.key(target: .r, substitution: .l)
        XCTAssertEqual(key, "r>l")
        let parsed = ErrorHistory.parse(key)
        XCTAssertEqual(parsed?.target, .r)
        XCTAssertEqual(parsed?.substitution, .l)
        XCTAssertNil(ErrorHistory.parse("garbage"))
    }
}

final class StageCatalogTests: XCTestCase {
    func testBundledCatalogIsValid() throws {
        let catalog = try StageCatalog.load()
        XCTAssertFalse(catalog.stages.isEmpty)
        XCTAssertEqual(Set(catalog.stages.map { $0.id }).count, catalog.stages.count, "stage ids must be unique")
        XCTAssertEqual(Set(catalog.stages.map { $0.number }).count, catalog.stages.count, "stage numbers must be unique")
        XCTAssertEqual(catalog.stages.filter { $0.isFree }.count, 1, "exactly one starter stage is free")
        for stage in catalog.stages {
            XCTAssertFalse(stage.words.isEmpty, stage.id)
            XCTAssertFalse(stage.cues.isEmpty, stage.id)
            XCTAssertTrue((0...1).contains(stage.mapPosition.x) && (0...1).contains(stage.mapPosition.y), stage.id)
            for word in stage.words {
                XCTAssertTrue(stage.sounds.contains(word.target), "\(word.text) target not in stage sounds")
            }
        }
    }
}

final class EngineTests: XCTestCase {
    private let word = PracticeWord(text: "right", ipa: "/raɪt/", target: .r, contrast: .l)

    func testUnavailableEngineNeverFabricatesAScore() async {
        let decision = await UnavailablePronunciationEngine().analyze(
            EngineRequest(word: word, samples: [0.5, 0.5], sampleRate: 16_000)
        )
        XCTAssertEqual(decision, .retry(.engineUnavailable))
    }

    func testPreviewEngineRejectsShortAndSilentAudio() async {
        let engine = PreviewPronunciationEngine()
        let tooShort = await engine.analyze(EngineRequest(word: word, samples: [0.1], sampleRate: 16_000))
        XCTAssertEqual(tooShort, .retry(.tooShort))
        let silent = await engine.analyze(EngineRequest(word: word, samples: Array(repeating: 0, count: 16_000), sampleRate: 16_000))
        XCTAssertEqual(silent, .retry(.silence))
    }
}
