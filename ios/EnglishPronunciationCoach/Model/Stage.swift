import Foundation

struct PracticeWord: Codable, Hashable, Identifiable {
    let text: String
    let ipa: String
    /// The sound the learner is trying to produce.
    let target: Sound
    /// The sound Japanese speakers commonly substitute, if any.
    let contrast: Sound?

    var id: String { text }
}

struct ArticulationCue: Codable, Hashable, Identifiable {
    /// 舌 / 唇 / 息 / 声
    let part: String
    let instruction: String

    var id: String { part }
}

/// Normalised (0...1) position of a stage on the adventure map.
struct MapPosition: Codable, Hashable {
    let x: Double
    let y: Double
}

struct Stage: Codable, Hashable, Identifiable {
    let id: String
    let number: Int
    let title: String
    let summary: String
    let sounds: [Sound]
    /// Free-tier stage. Everything else needs Pro.
    let isFree: Bool
    /// Shows the tongue-position diagram (only authored for /r/ vs /l/ so far).
    let showsRLDiagram: Bool
    let words: [PracticeWord]
    let cues: [ArticulationCue]
    let mapPosition: MapPosition

    var primarySound: Sound { sounds.first ?? .r }
}

/// Versioned lesson content, loaded from `stages.json` (never hard-coded in Swift).
struct StageCatalog: Codable {
    let version: Int
    let stages: [Stage]

    static let shared: StageCatalog = {
        (try? StageCatalog.load()) ?? StageCatalog(version: 0, stages: [])
    }()

    static func load(from bundle: Bundle = .main) throws -> StageCatalog {
        guard let url = bundle.url(forResource: "stages", withExtension: "json") else {
            throw CocoaError(.fileNoSuchFile)
        }
        return try JSONDecoder().decode(StageCatalog.self, from: Data(contentsOf: url))
    }
}
