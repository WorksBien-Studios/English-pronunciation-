import SwiftUI

struct StarShape: Shape {
    func path(in rect: CGRect) -> Path {
        let points: [(CGFloat, CGFloat)] = [
            (10, 1), (12.6, 7.2), (19.3, 7.6), (14.2, 12), (15.9, 18.6),
            (10, 15), (4.1, 18.6), (5.8, 12), (0.7, 7.6), (7.4, 7.2)
        ]
        let scaleX = rect.width / 20
        let scaleY = rect.height / 20
        var path = Path()
        for (index, point) in points.enumerated() {
            let mapped = CGPoint(x: rect.minX + point.0 * scaleX, y: rect.minY + point.1 * scaleY)
            if index == 0 { path.move(to: mapped) } else { path.addLine(to: mapped) }
        }
        path.closeSubpath()
        return path
    }
}

/// Three-star rating row (★1–3).
struct StarsView: View {
    let count: Int
    var size: CGFloat = 20

    var body: some View {
        HStack(spacing: 2) {
            ForEach(0..<3, id: \.self) { index in
                StarShape()
                    .fill(index < count ? Palette.sun : Color.white.opacity(0.75))
                    .overlay(StarShape().stroke(Palette.ink, style: StrokeStyle(lineWidth: 1.6, lineJoin: .round)))
                    .frame(width: size, height: size)
            }
        }
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("星\(count)つ")
    }
}
