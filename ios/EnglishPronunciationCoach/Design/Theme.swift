import SwiftUI
import UIKit

extension Color {
    /// App tint (藍): #2C4DAE in light mode, #7C9BFF in dark mode.
    static let brand = Color(uiColor: UIColor { traits in
        traits.userInterfaceStyle == .dark
            ? UIColor(red: 0.486, green: 0.608, blue: 1, alpha: 1)
            : UIColor(red: 0.173, green: 0.302, blue: 0.682, alpha: 1)
    })
}

/// Container width at which the layouts switch from one column to two (iPad Pro landscape, iPad Pro 13" portrait).
/// It is decided by the space the view actually gets, never by the device model, so Split View and Stage Manager work.
enum Layout {
    static let twoColumnMinWidth: CGFloat = 900
    static let sidebarWidth: CGFloat = 340
    /// Readable width for single-column content on regular-width screens.
    static let readableWidth: CGFloat = 620
}

/// Shows `wide` when the container is at least `threshold` points wide, otherwise `compact`.
struct WidthAdaptive<Wide: View, Compact: View>: View {
    let threshold: CGFloat
    let wide: Wide
    let compact: Compact

    init(
        threshold: CGFloat = Layout.twoColumnMinWidth,
        @ViewBuilder wide: () -> Wide,
        @ViewBuilder compact: () -> Compact
    ) {
        self.threshold = threshold
        self.wide = wide()
        self.compact = compact()
    }

    var body: some View {
        GeometryReader { geometry in
            if geometry.size.width >= threshold {
                wide
            } else {
                compact
            }
        }
    }
}

extension View {
    /// Grouped-list style card for content that lives outside a `List`.
    func card() -> some View {
        padding(16)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
    }

    /// Centres single-column content and stops it from stretching across a wide iPad.
    func readableColumn() -> some View {
        frame(maxWidth: Layout.readableWidth)
            .frame(maxWidth: .infinity)
    }
}
