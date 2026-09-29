import Foundation

/// The six launch sounds. Names are deliberately generic sound labels
/// (see docs/game-ui-concept.md), so they carry no brand risk.
enum Sound: String, CaseIterable, Identifiable, Codable, Hashable {
    case r, l, th, v, f, b

    var id: String { rawValue }

    var ipa: String {
        switch self {
        case .r: "/r/"
        case .l: "/l/"
        case .th: "/θ/"
        case .v: "/v/"
        case .f: "/f/"
        case .b: "/b/"
        }
    }

    /// Creature name shown in the 図鑑 and on cards.
    var displayName: String {
        switch self {
        case .r: "Rくん"
        case .l: "Lくん"
        case .th: "THくん"
        case .v: "Vくん"
        case .f: "Fくん"
        case .b: "Bくん"
        }
    }

    /// Body colour of the creature as 0xRRGGBB.
    var bodyHex: UInt32 {
        switch self {
        case .r: 0xFF7A59
        case .l: 0x54C2F0
        case .th: 0xC79BFF
        case .v: 0xFFB020
        case .f: 0x3DDBB3
        case .b: 0xFFF1D0
        }
    }
}
