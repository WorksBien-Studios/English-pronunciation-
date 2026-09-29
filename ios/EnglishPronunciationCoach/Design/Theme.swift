import SwiftUI

extension Color {
    /// 0xRRGGBB sRGB.
    init(hex: UInt32) {
        self.init(
            .sRGB,
            red: Double((hex >> 16) & 0xFF) / 255,
            green: Double((hex >> 8) & 0xFF) / 255,
            blue: Double(hex & 0xFF) / 255,
            opacity: 1
        )
    }
}

enum Palette {
    static let ink = Color(hex: 0x1B1F5E)
    static let indigo = Color(hex: 0x2A2F8C)
    static let sun = Color(hex: 0xFFC933)
    static let sunLight = Color(hex: 0xFFE066)
    static let sunEdge = Color(hex: 0xC48A00)
    static let coral = Color(hex: 0xE5372B)
    static let coralLight = Color(hex: 0xFF7A6B)
    static let coralEdge = Color(hex: 0x9E1F16)
    static let mint = Color(hex: 0x3DDBB3)
    static let teal = Color(hex: 0x12B5A4)
    static let skyTop = Color(hex: 0x5CC6F5)
    static let skyMid = Color(hex: 0x9BE3FF)
    static let skyBottom = Color(hex: 0xBDEEFF)
    static let paleSky = Color(hex: 0xE4F6FF)
    static let locked = Color(hex: 0xC9CDE0)
    static let lockedEdge = Color(hex: 0x7D84AD)
    static let secondaryText = Color(hex: 0x4A4F80)
    static let cheek = Color(hex: 0xFFB3C7)
    static let tonguePink = Color(hex: 0xFF8FA3)
    static let mouthDark = Color(hex: 0x7A1F10)
    static let grass = Color(hex: 0x82DC5A)
    static let sand = Color(hex: 0xFFE7A3)
}

extension Font {
    /// Hiragino Maru Gothic (a system font on iOS) scaled with Dynamic Type.
    static func game(_ size: CGFloat, relativeTo style: Font.TextStyle = .body) -> Font {
        .custom("HiraMaruProN-W4", size: size, relativeTo: style)
    }
}

/// Sky-to-water backdrop shared by the game screens.
struct SkyBackground: View {
    var body: some View {
        LinearGradient(
            colors: [Palette.skyTop, Palette.skyMid, Palette.skyBottom],
            startPoint: .top,
            endPoint: .bottom
        )
        .ignoresSafeArea()
    }
}

/// White card with the heavy ink outline and hard bottom edge used across the game UI.
struct GameCardModifier: ViewModifier {
    var radius: CGFloat = 22

    func body(content: Content) -> some View {
        content
            .background(
                RoundedRectangle(cornerRadius: radius, style: .continuous).fill(Color.white)
            )
            .overlay(
                RoundedRectangle(cornerRadius: radius, style: .continuous)
                    .stroke(Palette.ink, lineWidth: 3)
            )
            .shadow(color: Palette.ink, radius: 0, x: 0, y: 4)
    }
}

extension View {
    func gameCard(radius: CGFloat = 22) -> some View {
        modifier(GameCardModifier(radius: radius))
    }
}
