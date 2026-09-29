import Foundation
import SwiftUI

/// Game-style display text: coloured fill with a solid outline, built from layered `Text`.
struct OutlinedText: View {
    let text: String
    var size: CGFloat = 24
    var fill: Color = .white
    var stroke: Color = Palette.ink
    var lineWidth: CGFloat = 4

    var body: some View {
        ZStack {
            ForEach(0..<8, id: \.self) { index in
                let angle = Double(index) * Double.pi / 4
                Text(text)
                    .foregroundStyle(stroke)
                    .offset(x: CGFloat(cos(angle)) * lineWidth / 2, y: CGFloat(sin(angle)) * lineWidth / 2)
            }
            Text(text).foregroundStyle(fill)
        }
        .font(.game(size, relativeTo: .title2))
        .accessibilityElement(children: .ignore)
        .accessibilityLabel(text)
    }
}
