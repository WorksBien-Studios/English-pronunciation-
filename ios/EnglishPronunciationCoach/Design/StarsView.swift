import SwiftUI

/// Three-star rating row (★1–3) made from SF Symbols.
struct StarsView: View {
    let count: Int
    var size: CGFloat = 15

    var body: some View {
        HStack(spacing: 1) {
            ForEach(0..<3, id: \.self) { index in
                Image(systemName: index < count ? "star.fill" : "star")
                    .font(.system(size: size))
                    .foregroundStyle(index < count ? Color.yellow : Color.secondary.opacity(0.5))
            }
        }
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("星\(count)つ")
    }
}
