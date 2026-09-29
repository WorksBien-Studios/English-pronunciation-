import SwiftUI

/// A collectible sound creature, drawn as vector art so it stays sharp at any size.
///
/// Mouth states follow Japanese character conventions (docs/game-ui-concept.md §6a):
/// - `.rest`: small closed "ω" smile (default)
/// - `.show`: the articulation cue, only on cue screens
/// - `.joy`: closed "^ ^" eyes and a small open smile, celebration only
struct CreatureView: View {
    enum Mouth {
        case rest, show, joy
    }

    let sound: Sound
    var mouth: Mouth = .rest
    var isLocked = false

    var body: some View {
        Canvas { context, size in
            var ctx = context
            let scale = min(size.width, size.height) / 100
            ctx.translateBy(x: (size.width - 100 * scale) / 2, y: (size.height - 100 * scale) / 2)
            ctx.scaleBy(x: scale, y: scale)
            CreatureArt(sound: sound, mouth: mouth, isLocked: isLocked).draw(in: &ctx)
        }
        .aspectRatio(1, contentMode: .fit)
        .accessibilityElement(children: .ignore)
        .accessibilityLabel(isLocked ? "まだ出会っていない仲間" : sound.displayName)
    }
}

private struct CreatureArt {
    let sound: Sound
    let mouth: CreatureView.Mouth
    let isLocked: Bool

    private var ink: Color { Palette.ink }
    private var body: Color { isLocked ? Palette.locked : Color(hex: sound.bodyHex) }
    private var accent: Color { isLocked ? Color(hex: 0xDDE0EE) : Palette.tonguePink }
    private var leaf: Color { isLocked ? Color(hex: 0xDDE0EE) : Color(hex: 0xA6F3DF) }

    func draw(in ctx: inout GraphicsContext) {
        Art.fill(Art.oval(50, 94, 27, 4), ink.opacity(0.18), &ctx)
        drawSilhouetteFeature(&ctx)
        Art.fillAndStroke(Art.oval(50, 57, 37, 34), fill: body, width: 3.5, &ctx)

        if isLocked {
            ctx.draw(
                Text("?").font(.system(size: 44, weight: .bold)).foregroundStyle(Color(hex: 0xF4F5FB)),
                at: CGPoint(x: 50, y: 62)
            )
            return
        }
        drawFace(&ctx)
        switch mouth {
        case .rest: drawRestMouth(&ctx)
        case .show: drawShowMouth(&ctx)
        case .joy: drawJoy(&ctx)
        }
    }

    // MARK: Silhouette (behind the body, also visible when locked)

    private func drawSilhouetteFeature(_ ctx: inout GraphicsContext) {
        switch sound {
        case .r:
            var curl = Path()
            curl.move(to: Art.point(50, 26))
            curl.addCurve(to: Art.point(62, 14), control1: Art.point(42, 12), control2: Art.point(60, 2))
            curl.addCurve(to: Art.point(54, 16), control1: Art.point(63, 22), control2: Art.point(54, 22))
            Art.stroke(curl, ink, 4, &ctx)
        case .l:
            Art.stroke(Art.line(Art.point(50, 26), Art.point(50, 14)), ink, 4, &ctx)
            Art.fillAndStroke(Art.circle(50, 10, 6.2), fill: accent, width: 3, &ctx)
        case .th:
            Art.fillAndStroke(Art.circle(24, 30, 9.5), fill: body, width: 3.5, &ctx)
            Art.fillAndStroke(Art.circle(76, 30, 9.5), fill: body, width: 3.5, &ctx)
        case .v:
            Art.fillAndStroke(
                Art.triangle(Art.point(20, 40), Art.point(17, 15), Art.point(40, 28)),
                fill: body, width: 3.5, &ctx
            )
            Art.fillAndStroke(
                Art.triangle(Art.point(80, 40), Art.point(83, 15), Art.point(60, 28)),
                fill: body, width: 3.5, &ctx
            )
        case .f:
            var leafPath = Path()
            leafPath.move(to: Art.point(50, 26))
            leafPath.addCurve(to: Art.point(58, 5), control1: Art.point(39, 16), control2: Art.point(44, 3))
            leafPath.addCurve(to: Art.point(50, 26), control1: Art.point(61, 16), control2: Art.point(57, 23))
            leafPath.closeSubpath()
            Art.fillAndStroke(leafPath, fill: leaf, width: 3.2, &ctx)
            var vein = Path()
            vein.move(to: Art.point(51, 22))
            vein.addCurve(to: Art.point(56, 8), control1: Art.point(51, 17), control2: Art.point(53, 12))
            Art.stroke(vein, ink, 2, &ctx)
        case .b:
            Art.fillAndStroke(Art.circle(58, 13, 8), fill: Color.white.opacity(0.85), width: 3, &ctx)
            Art.stroke(Art.quad(Art.point(54.5, 11), Art.point(58.5, 8), control: Art.point(56, 8)), ink, 1.8, &ctx)
            Art.fillAndStroke(Art.circle(43, 9, 3.6), fill: Color.white.opacity(0.85), width: 2.4, &ctx)
        }
    }

    // MARK: Face

    private func drawFace(_ ctx: inout GraphicsContext) {
        var shine = Art.oval(34, 38, 11, 6)
        let rotation = CGAffineTransform(translationX: 34, y: 38)
            .rotated(by: -24 * .pi / 180)
            .translatedBy(x: -34, y: -38)
        shine = shine.applying(rotation)
        Art.fill(shine, Color.white.opacity(0.38), &ctx)

        // Solid glossy eyes with two highlights (きらきら).
        Art.fill(Art.oval(36, 53, 6.4, 7.6), ink, &ctx)
        Art.fill(Art.oval(64, 53, 6.4, 7.6), ink, &ctx)
        Art.fill(Art.circle(38.2, 50, 2.7), .white, &ctx)
        Art.fill(Art.circle(66.2, 50, 2.7), .white, &ctx)
        Art.fill(Art.circle(34.2, 57.2, 1.3), .white, &ctx)
        Art.fill(Art.circle(62.2, 57.2, 1.3), .white, &ctx)

        Art.fill(Art.oval(24, 66, 6, 4), Palette.cheek, &ctx)
        Art.fill(Art.oval(76, 66, 6, 4), Palette.cheek, &ctx)
    }

    // MARK: Mouths

    private func drawRestMouth(_ ctx: inout GraphicsContext) {
        var omega = Path()
        omega.move(to: Art.point(43, 67))
        omega.addQuadCurve(to: Art.point(50, 67), control: Art.point(46.5, 71.5))
        omega.addQuadCurve(to: Art.point(57, 67), control: Art.point(53.5, 71.5))
        Art.stroke(omega, ink, 3, &ctx)

        switch sound {
        case .th:
            // てへぺろ: wink plus a small side tongue.
            Art.fill(Art.circle(64, 53, 10.5), body, &ctx)
            Art.stroke(Art.quad(Art.point(56, 55), Art.point(72, 55), control: Art.point(64, 46)), ink, 3.6, &ctx)
            Art.fillAndStroke(Art.oval(58, 73.5, 3.6, 4.6), fill: Palette.tonguePink, width: 2, &ctx)
        case .v:
            // 八重歯: a single small fang.
            Art.fillAndStroke(
                Art.triangle(Art.point(43.5, 68.5), Art.point(48, 68.5), Art.point(45.8, 74)),
                fill: .white, width: 1.8, &ctx
            )
        case .b:
            // Puffed cheeks.
            Art.fill(Art.oval(22, 67, 8, 6), Palette.cheek, &ctx)
            Art.fill(Art.oval(78, 67, 8, 6), Palette.cheek, &ctx)
        default:
            break
        }
    }

    private func drawJoy(_ ctx: inout GraphicsContext) {
        // Closed "^ ^" eyes over the normal ones.
        Art.fill(Art.circle(36, 53, 10.5), body, &ctx)
        Art.fill(Art.circle(64, 53, 10.5), body, &ctx)
        Art.stroke(Art.quad(Art.point(28, 55), Art.point(44, 55), control: Art.point(36, 45)), ink, 3.6, &ctx)
        Art.stroke(Art.quad(Art.point(56, 55), Art.point(72, 55), control: Art.point(64, 45)), ink, 3.6, &ctx)

        var smile = Path()
        smile.move(to: Art.point(42, 63))
        smile.addQuadCurve(to: Art.point(58, 63), control: Art.point(50, 78))
        smile.closeSubpath()
        Art.fillAndStroke(smile, fill: Palette.mouthDark, width: 2.4, &ctx)
        Art.fill(Art.oval(50, 71.5, 4.5, 2.8), Palette.tonguePink, &ctx)
    }

    private func drawShowMouth(_ ctx: inout GraphicsContext) {
        switch sound {
        case .r:
            Art.fillAndStroke(Art.oval(50, 69, 4.5, 5.5), fill: Palette.mouthDark, width: 2, &ctx)
        case .l:
            var open = Path()
            open.move(to: Art.point(41, 65))
            open.addQuadCurve(to: Art.point(59, 65), control: Art.point(50, 76))
            open.closeSubpath()
            Art.fillAndStroke(open, fill: Palette.mouthDark, width: 2, &ctx)
            Art.fill(Art.oval(50, 66.5, 5, 3), Palette.tonguePink, &ctx)
        case .th:
            let teeth = Path(roundedRect: CGRect(x: 40, y: 65, width: 20, height: 6), cornerRadius: 2)
            Art.fillAndStroke(teeth, fill: .white, width: 2, &ctx)
            Art.fillAndStroke(Art.oval(50, 73, 7.5, 6), fill: Palette.tonguePink, width: 2, &ctx)
            Art.stroke(Art.line(Art.point(50, 70), Art.point(50, 76)), Color(hex: 0xD9587A), 1.6, &ctx)
        case .v:
            Art.stroke(Art.quad(Art.point(40, 68), Art.point(60, 68), control: Art.point(50, 75)), ink, 3, &ctx)
            Art.fillAndStroke(
                Art.triangle(Art.point(43, 66), Art.point(48, 66), Art.point(45.5, 72.5)),
                fill: .white, width: 2, &ctx
            )
            Art.fillAndStroke(
                Art.triangle(Art.point(52, 66), Art.point(57, 66), Art.point(54.5, 72.5)),
                fill: .white, width: 2, &ctx
            )
        case .f:
            Art.fill(Art.oval(50, 69, 3.8, 4.4), ink, &ctx)
            Art.stroke(Art.quad(Art.point(60, 69), Art.point(74, 67), control: Art.point(68, 70)), .white, 3, &ctx)
            Art.stroke(Art.quad(Art.point(60, 74), Art.point(75, 75), control: Art.point(69, 77)), .white, 3, &ctx)
        case .b:
            Art.fillAndStroke(Art.oval(50, 69, 8, 3.2), fill: Color(hex: 0xC2185B), width: 2, &ctx)
            Art.fill(Art.oval(22, 67, 8, 6), Palette.cheek, &ctx)
            Art.fill(Art.oval(78, 67, 8, 6), Palette.cheek, &ctx)
        }
    }
}

#Preview("Creatures") {
    ScrollView {
        LazyVGrid(columns: [GridItem(.adaptive(minimum: 90))]) {
            ForEach(Sound.allCases) { sound in
                VStack {
                    CreatureView(sound: sound).frame(width: 80, height: 80)
                    CreatureView(sound: sound, mouth: .show).frame(width: 80, height: 80)
                    CreatureView(sound: sound, mouth: .joy).frame(width: 80, height: 80)
                    CreatureView(sound: sound, isLocked: true).frame(width: 80, height: 80)
                }
            }
        }
        .padding()
    }
    .background(Color.white)
}
