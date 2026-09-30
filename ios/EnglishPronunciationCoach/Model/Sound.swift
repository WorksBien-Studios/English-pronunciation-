import Foundation

/// The six launch sounds.
enum Sound: String, CaseIterable, Identifiable, Codable, Hashable {
    case r, l, th, v, f, b

    var id: String { rawValue }

    /// The IPA symbol on its own, e.g. "θ".
    var symbol: String {
        switch self {
        case .r: "r"
        case .l: "l"
        case .th: "θ"
        case .v: "v"
        case .f: "f"
        case .b: "b"
        }
    }

    /// IPA in slashes, e.g. "/θ/".
    var ipa: String { "/\(symbol)/" }

    /// Short Latin label used in lists and headings.
    var label: String {
        switch self {
        case .r: "R"
        case .l: "L"
        case .th: "TH"
        case .v: "V"
        case .f: "F"
        case .b: "B"
        }
    }

    /// The Japanese sound learners usually replace it with.
    var japaneseNote: String {
        switch self {
        case .r: "日本語のラ行に聞こえやすい音"
        case .l: "日本語のラ行になりやすい音"
        case .th: "サ行やザ行になりやすい音"
        case .v: "バ行になりやすい音"
        case .f: "ハ行になりやすい音"
        case .b: "Vと混ざりやすい音"
        }
    }
}
