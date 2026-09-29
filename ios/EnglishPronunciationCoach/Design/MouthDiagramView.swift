import SwiftUI

/// Side-view mouth diagram: the English /r/ tongue (solid) against the Japanese ラ行 tongue (dashed).
struct MouthDiagramView: View {
    var body: some View {
        Canvas { context, size in
            var ctx = context
            let scale = size.width / 320
            ctx.scaleBy(x: scale, y: scale)
            draw(in: &ctx)
        }
        .aspectRatio(320.0 / 200.0, contentMode: .fit)
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("口の断面図。英語の/r/は舌先が上あごにつかない。日本語のラ行は舌先が上あごに触れる。")
    }

    private func draw(in ctx: inout GraphicsContext) {
        let ink = Palette.ink
        let frame = Path(roundedRect: CGRect(x: 1.5, y: 1.5, width: 317, height: 197), cornerRadius: 22)
        Art.fillAndStroke(frame, fill: Palette.paleSky, width: 3, &ctx)

        drawPill(&ctx, rect: CGRect(x: 14, y: 10, width: 136, height: 26), fill: Palette.teal, dashed: false, stroke: ink)
        ctx.draw(
            Text("英語の/r/：つけない").font(.system(size: 13, weight: .heavy)).foregroundStyle(Color.white),
            at: CGPoint(x: 82, y: 23)
        )
        drawPill(&ctx, rect: CGRect(x: 160, y: 10, width: 146, height: 26), fill: Color(hex: 0xFFE9E4), dashed: true, stroke: Color(hex: 0xC8402B))
        ctx.draw(
            Text("日本語のラ：触れる").font(.system(size: 13, weight: .heavy)).foregroundStyle(Color(hex: 0x8A2A18)),
            at: CGPoint(x: 233, y: 23)
        )

        ctx.translateBy(x: 8, y: 34)

        // Mouth cavity.
        var cavity = Path()
        cavity.move(to: Art.point(252, 72))
        cavity.addQuadCurve(to: Art.point(230, 64), control: Art.point(246, 60))
        cavity.addQuadCurve(to: Art.point(150, 44), control: Art.point(200, 42))
        cavity.addQuadCurve(to: Art.point(78, 78), control: Art.point(100, 46))
        cavity.addLine(to: Art.point(70, 140))
        cavity.addQuadCurve(to: Art.point(170, 140), control: Art.point(110, 148))
        cavity.addQuadCurve(to: Art.point(232, 116), control: Art.point(212, 130))
        cavity.addQuadCurve(to: Art.point(252, 100), control: Art.point(246, 112))
        cavity.closeSubpath()
        Art.fill(cavity, Color(hex: 0xFFF3EE), &ctx)

        var palate = Path()
        palate.move(to: Art.point(252, 72))
        palate.addQuadCurve(to: Art.point(230, 64), control: Art.point(246, 60))
        palate.addQuadCurve(to: Art.point(150, 44), control: Art.point(200, 42))
        palate.addQuadCurve(to: Art.point(78, 78), control: Art.point(100, 46))
        palate.addLine(to: Art.point(70, 140))
        Art.stroke(palate, ink, 3.5, &ctx)

        var floor = Path()
        floor.move(to: Art.point(252, 100))
        floor.addQuadCurve(to: Art.point(232, 116), control: Art.point(246, 112))
        floor.addQuadCurve(to: Art.point(170, 140), control: Art.point(212, 130))
        floor.addQuadCurve(to: Art.point(70, 140), control: Art.point(110, 148))
        Art.stroke(floor, ink, 3.5, &ctx)

        // Lips and front tooth.
        var upperLip = Path()
        upperLip.move(to: Art.point(252, 62))
        upperLip.addQuadCurve(to: Art.point(266, 82), control: Art.point(270, 66))
        upperLip.addQuadCurve(to: Art.point(250, 80), control: Art.point(258, 88))
        upperLip.closeSubpath()
        Art.fillAndStroke(upperLip, fill: Color(hex: 0xFF9DB5), width: 2.5, &ctx)
        var lowerLip = Path()
        lowerLip.move(to: Art.point(252, 96))
        lowerLip.addQuadCurve(to: Art.point(268, 114), control: Art.point(270, 98))
        lowerLip.addQuadCurve(to: Art.point(248, 112), control: Art.point(258, 124))
        lowerLip.closeSubpath()
        Art.fillAndStroke(lowerLip, fill: Color(hex: 0xFF9DB5), width: 2.5, &ctx)
        Art.fillAndStroke(Path(roundedRect: CGRect(x: 243, y: 66, width: 7, height: 16), cornerRadius: 2.5), fill: .white, width: 2, &ctx)

        // English /r/ tongue (solid).
        var tongue = Path()
        tongue.move(to: Art.point(84, 138))
        tongue.addCurve(to: Art.point(136, 92), control1: Art.point(88, 108), control2: Art.point(108, 92))
        tongue.addCurve(to: Art.point(194, 90), control1: Art.point(156, 92), control2: Art.point(176, 88))
        tongue.addCurve(to: Art.point(196, 104), control1: Art.point(206, 91), control2: Art.point(208, 100))
        tongue.addCurve(to: Art.point(158, 140), control1: Art.point(176, 112), control2: Art.point(164, 124))
        tongue.addCurve(to: Art.point(84, 138), control1: Art.point(140, 146), control2: Art.point(104, 146))
        tongue.closeSubpath()
        Art.fillAndStroke(tongue, fill: Palette.teal, width: 3, &ctx)

        // Japanese ラ行 tongue (dashed): the tip touches the ridge behind the teeth.
        var japanese = Path()
        japanese.move(to: Art.point(84, 138))
        japanese.addCurve(to: Art.point(170, 96), control1: Art.point(96, 118), control2: Art.point(130, 108))
        japanese.addCurve(to: Art.point(228, 68), control1: Art.point(196, 88), control2: Art.point(216, 74))
        japanese.addCurve(to: Art.point(218, 92), control1: Art.point(234, 72), control2: Art.point(230, 82))
        japanese.addCurve(to: Art.point(160, 140), control1: Art.point(196, 108), control2: Art.point(176, 122))
        ctx.stroke(
            japanese,
            with: .color(Color(hex: 0xE5482D)),
            style: StrokeStyle(lineWidth: 3, lineCap: .round, dash: [7, 5])
        )
        Art.fillAndStroke(Art.circle(228, 67, 5.5), fill: Color(hex: 0xE5482D), stroke: .white, width: 2, &ctx)

        ctx.draw(Text("舌").font(.system(size: 14, weight: .heavy)).foregroundStyle(ink), at: CGPoint(x: 126, y: 124))
        ctx.draw(Text("上あご").font(.system(size: 12, weight: .heavy)).foregroundStyle(ink), at: CGPoint(x: 140, y: 34))
    }

    private func drawPill(_ ctx: inout GraphicsContext, rect: CGRect, fill: Color, dashed: Bool, stroke: Color) {
        let path = Path(roundedRect: rect, cornerRadius: rect.height / 2)
        Art.fill(path, fill, &ctx)
        ctx.stroke(
            path,
            with: .color(stroke),
            style: StrokeStyle(lineWidth: 2.5, dash: dashed ? [6, 4] : [])
        )
    }
}

#Preview("Mouth diagram") {
    MouthDiagramView().padding()
}
