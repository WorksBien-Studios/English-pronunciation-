import SwiftUI

/// The guide character: a speech bubble with a wavy hair strand.
struct CoachView: View {
    enum Mood {
        case happy, cheer
    }

    var mood: Mood = .happy

    var body: some View {
        Canvas { context, size in
            var ctx = context
            let scale = min(size.width, size.height) / 100
            ctx.translateBy(x: (size.width - 100 * scale) / 2, y: (size.height - 100 * scale) / 2)
            ctx.scaleBy(x: scale, y: scale)
            draw(in: &ctx)
        }
        .aspectRatio(1, contentMode: .fit)
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("コーチ")
    }

    private func draw(in ctx: inout GraphicsContext) {
        let ink = Palette.ink
        Art.fill(Art.oval(50, 95, 26, 3.6), ink.opacity(0.18), &ctx)

        let tail = Art.triangle(Art.point(24, 74), Art.point(12, 92), Art.point(42, 80))
        Art.fillAndStroke(tail, fill: .white, width: 3.5, &ctx)
        let bubble = Path(roundedRect: CGRect(x: 8, y: 16, width: 84, height: 64), cornerRadius: 30)
        Art.fillAndStroke(bubble, fill: .white, width: 3.5, &ctx)
        Art.stroke(Art.line(Art.point(24, 74), Art.point(40, 79)), .white, 6, &ctx)

        var hair = Path()
        hair.move(to: Art.point(34, 16))
        hair.addQuadCurve(to: Art.point(44, 16), control: Art.point(39, 2))
        hair.addQuadCurve(to: Art.point(54, 16), control: Art.point(49, 30))
        hair.addQuadCurve(to: Art.point(64, 16), control: Art.point(59, 2))
        Art.stroke(hair, Palette.teal, 5, &ctx)

        switch mood {
        case .happy:
            Art.stroke(Art.quad(Art.point(28, 50), Art.point(40, 50), control: Art.point(34, 42)), ink, 4, &ctx)
            Art.stroke(Art.quad(Art.point(60, 50), Art.point(72, 50), control: Art.point(66, 42)), ink, 4, &ctx)
            Art.stroke(Art.quad(Art.point(42, 60), Art.point(58, 60), control: Art.point(50, 70)), ink, 4, &ctx)
        case .cheer:
            var leftEye = Path()
            leftEye.move(to: Art.point(27, 44))
            leftEye.addLine(to: Art.point(37, 50))
            leftEye.addLine(to: Art.point(27, 56))
            Art.stroke(leftEye, ink, 4, &ctx)
            var rightEye = Path()
            rightEye.move(to: Art.point(73, 44))
            rightEye.addLine(to: Art.point(63, 50))
            rightEye.addLine(to: Art.point(73, 56))
            Art.stroke(rightEye, ink, 4, &ctx)
            var mouth = Path()
            mouth.move(to: Art.point(43, 60))
            mouth.addQuadCurve(to: Art.point(57, 60), control: Art.point(50, 71))
            mouth.closeSubpath()
            Art.fillAndStroke(mouth, fill: ink, width: 2, &ctx)
            Art.stroke(Art.quad(Art.point(47, 64.5), Art.point(53, 64.5), control: Art.point(50, 66.9)), Palette.tonguePink, 3, &ctx)
        }
        Art.fill(Art.oval(22, 60, 6.5, 4.2), Palette.cheek, &ctx)
        Art.fill(Art.oval(78, 60, 6.5, 4.2), Palette.cheek, &ctx)
    }
}

#Preview("Coach") {
    HStack {
        CoachView(mood: .happy).frame(width: 96, height: 96)
        CoachView(mood: .cheer).frame(width: 96, height: 96)
    }
    .padding()
}
