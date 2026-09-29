import SwiftUI

/// The glossy, outlined, hard-edged button used for primary actions.
struct ChunkyButtonStyle: ButtonStyle {
    enum Tone {
        case primary, secondary, danger
    }

    var tone: Tone = .primary

    private var top: Color {
        switch tone {
        case .primary: Palette.sunLight
        case .secondary: .white
        case .danger: Palette.coralLight
        }
    }

    private var bottom: Color {
        switch tone {
        case .primary: Palette.sun
        case .secondary: .white
        case .danger: Palette.coral
        }
    }

    private var edge: Color {
        switch tone {
        case .primary: Palette.sunEdge
        case .secondary: Palette.lockedEdge
        case .danger: Palette.coralEdge
        }
    }

    private var foreground: Color {
        tone == .danger ? .white : Palette.ink
    }

    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.game(19, relativeTo: .headline))
            .foregroundStyle(foreground)
            .frame(maxWidth: .infinity, minHeight: 54)
            .background(
                RoundedRectangle(cornerRadius: 27, style: .continuous)
                    .fill(LinearGradient(colors: [top, bottom], startPoint: .top, endPoint: .bottom))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 27, style: .continuous)
                    .stroke(Palette.ink, lineWidth: 3)
            )
            .shadow(color: edge, radius: 0, x: 0, y: configuration.isPressed ? 2 : 6)
            .offset(y: configuration.isPressed ? 4 : 0)
            .animation(.easeOut(duration: 0.08), value: configuration.isPressed)
            .sensoryFeedback(.impact(weight: .light), trigger: configuration.isPressed)
    }
}

extension ButtonStyle where Self == ChunkyButtonStyle {
    static var chunky: ChunkyButtonStyle { ChunkyButtonStyle() }
}
