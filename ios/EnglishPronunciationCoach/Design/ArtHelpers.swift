import SwiftUI

/// Small drawing helpers for the vector character art (100×100 design space).
enum Art {
    static func point(_ x: CGFloat, _ y: CGFloat) -> CGPoint { CGPoint(x: x, y: y) }

    static func oval(_ cx: CGFloat, _ cy: CGFloat, _ rx: CGFloat, _ ry: CGFloat) -> Path {
        Path(ellipseIn: CGRect(x: cx - rx, y: cy - ry, width: rx * 2, height: ry * 2))
    }

    static func circle(_ cx: CGFloat, _ cy: CGFloat, _ r: CGFloat) -> Path {
        oval(cx, cy, r, r)
    }

    static func triangle(_ a: CGPoint, _ b: CGPoint, _ c: CGPoint) -> Path {
        var path = Path()
        path.move(to: a)
        path.addLine(to: b)
        path.addLine(to: c)
        path.closeSubpath()
        return path
    }

    /// Open quadratic curve from `start` to `end` with `control`.
    static func quad(_ start: CGPoint, _ end: CGPoint, control: CGPoint) -> Path {
        var path = Path()
        path.move(to: start)
        path.addQuadCurve(to: end, control: control)
        return path
    }

    static func line(_ start: CGPoint, _ end: CGPoint) -> Path {
        var path = Path()
        path.move(to: start)
        path.addLine(to: end)
        return path
    }

    static func fill(_ path: Path, _ color: Color, _ ctx: inout GraphicsContext) {
        ctx.fill(path, with: .color(color))
    }

    static func stroke(_ path: Path, _ color: Color, _ width: CGFloat, _ ctx: inout GraphicsContext) {
        ctx.stroke(
            path,
            with: .color(color),
            style: StrokeStyle(lineWidth: width, lineCap: .round, lineJoin: .round)
        )
    }

    static func fillAndStroke(
        _ path: Path,
        fill fillColor: Color,
        stroke strokeColor: Color = Palette.ink,
        width: CGFloat,
        _ ctx: inout GraphicsContext
    ) {
        fill(path, fillColor, &ctx)
        stroke(path, strokeColor, width, &ctx)
    }
}
