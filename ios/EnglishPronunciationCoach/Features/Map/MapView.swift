import SwiftData
import SwiftUI

enum StageNodeState: Equatable {
    case cleared(Int)
    case current
    case locked
    case proOnly
}

/// 冒険 tab: the island map. Compact width pushes the stage; regular width (iPad)
/// shows it in an inspector beside the map.
struct MapView: View {
    @Environment(\.horizontalSizeClass) private var horizontalSizeClass
    @Environment(EntitlementStore.self) private var entitlements
    @Query private var progress: [StageProgress]
    @Query private var usage: [DailyUsage]

    @State private var pushedStage: Stage?
    @State private var inspectedStage: Stage?
    @State private var showsPaywall = false
    @State private var showsOmikuji = false

    private let catalog = StageCatalog.shared

    private var isRegular: Bool { horizontalSizeClass == .regular }

    private var totalStars: Int { progress.reduce(0) { $0 + $1.stars } }

    private var usedToday: Int {
        usage.first { $0.day == DailyAllowance.dayKey(.now) }?.count ?? 0
    }

    private func stars(for stage: Stage) -> Int {
        progress.first { $0.stageID == stage.id }?.stars ?? 0
    }

    private func nodeState(for stage: Stage) -> StageNodeState {
        let earned = stars(for: stage)
        if earned > 0 { return .cleared(earned) }
        if !stage.isFree && !entitlements.isPro { return .proOnly }
        if let previous = catalog.stages.first(where: { $0.number == stage.number - 1 }),
           stars(for: previous) == 0 {
            return .locked
        }
        return .current
    }

    private func select(_ stage: Stage) {
        switch nodeState(for: stage) {
        case .proOnly:
            showsPaywall = true
        case .locked:
            break
        case .cleared, .current:
            if isRegular {
                inspectedStage = stage
            } else {
                pushedStage = stage
            }
        }
    }

    private var inspectorBinding: Binding<Bool> {
        Binding(
            get: { isRegular && inspectedStage != nil },
            set: { if !$0 { inspectedStage = nil } }
        )
    }

    var body: some View {
        ZStack {
            SkyBackground()
            IslandsView()
            if catalog.stages.isEmpty {
                ContentUnavailableView(
                    "ステージを読み込めません",
                    systemImage: "exclamationmark.triangle",
                    description: Text("アプリを再起動してください。")
                )
            } else {
                mapContent
            }
        }
        .safeAreaInset(edge: .top, spacing: 0) {
            MapHUD(
                totalStars: totalStars,
                remaining: DailyAllowance.remaining(used: usedToday, isPro: entitlements.isPro),
                onOmikuji: { showsOmikuji = true }
            )
        }
        .navigationTitle("冒険")
        .navigationBarTitleDisplayMode(.inline)
        .toolbarBackground(.hidden, for: .navigationBar)
        .navigationDestination(item: $pushedStage) { stage in
            StageDetailView(stage: stage)
        }
        .inspector(isPresented: inspectorBinding) {
            if let stage = inspectedStage {
                StageDetailView(stage: stage)
                    .inspectorColumnWidth(min: 320, ideal: 380, max: 460)
            }
        }
        .sheet(isPresented: $showsPaywall) { PaywallView() }
        .sheet(isPresented: $showsOmikuji) { OmikujiView() }
    }

    private var mapContent: some View {
        GeometryReader { geometry in
            let nodeSize: CGFloat = isRegular ? 80 : 64
            let points = catalog.stages.map {
                CGPoint(x: $0.mapPosition.x * geometry.size.width, y: $0.mapPosition.y * geometry.size.height)
            }
            ZStack {
                RoadShape(points: points)
                    .stroke(Color.white, style: StrokeStyle(lineWidth: 8, lineCap: .round, dash: [1, 14]))
                ForEach(Array(catalog.stages.enumerated()), id: \.element.id) { index, stage in
                    StageNode(stage: stage, state: nodeState(for: stage), size: nodeSize) {
                        select(stage)
                    }
                    .position(points[index])
                }
            }
        }
        .padding(.vertical, 24)
    }
}

private struct RoadShape: Shape {
    let points: [CGPoint]

    func path(in rect: CGRect) -> Path {
        var path = Path()
        guard let first = points.first else { return path }
        path.move(to: first)
        var previous = first
        for point in points.dropFirst() {
            let mid = CGPoint(x: (previous.x + point.x) / 2, y: (previous.y + point.y) / 2)
            path.addQuadCurve(to: mid, control: previous)
            previous = point
        }
        path.addLine(to: previous)
        return path
    }
}

private struct IslandsView: View {
    var body: some View {
        Canvas { context, size in
            var ctx = context
            let islands: [(CGFloat, CGFloat, CGFloat, CGFloat)] = [
                (0.47, 0.78, 0.40, 0.105),
                (0.55, 0.55, 0.40, 0.10),
                (0.44, 0.33, 0.40, 0.11)
            ]
            for island in islands {
                let center = CGPoint(x: island.0 * size.width, y: island.1 * size.height)
                let rx = island.2 * size.width
                let ry = island.3 * size.height
                let sand = Path(ellipseIn: CGRect(x: center.x - rx, y: center.y - ry, width: rx * 2, height: ry * 2))
                let grass = Path(ellipseIn: CGRect(x: center.x - rx * 0.89, y: center.y - ry * 0.87, width: rx * 1.78, height: ry * 1.74))
                Art.fillAndStroke(sand, fill: Palette.sand, width: 3.5, &ctx)
                Art.fillAndStroke(grass, fill: Palette.grass, width: 3, &ctx)
            }
        }
        .accessibilityHidden(true)
    }
}

private struct MapHUD: View {
    let totalStars: Int
    /// `nil` = unlimited (Pro).
    let remaining: Int?
    let onOmikuji: () -> Void

    var body: some View {
        HStack(spacing: 10) {
            VStack(spacing: 0) {
                Text("Lv").font(.game(10, relativeTo: .caption2))
                Text("\(PlayerLevel.level(totalStars: totalStars))").font(.game(20, relativeTo: .title3))
            }
            .foregroundStyle(Palette.ink)
            .frame(width: 50, height: 50)
            .background(Circle().fill(Palette.sun))
            .overlay(Circle().stroke(Palette.ink, lineWidth: 3))
            .accessibilityElement(children: .combine)

            VStack(alignment: .leading, spacing: 3) {
                OutlinedText(
                    text: "つぎのレベルまで あと\(PlayerLevel.expToNextLevel(totalStars: totalStars)) EXP",
                    size: 11,
                    lineWidth: 3
                )
                ProgressBar(fraction: PlayerLevel.fractionToNextLevel(totalStars: totalStars))
                    .frame(height: 14)
            }
            .frame(maxWidth: .infinity, alignment: .leading)

            HStack(spacing: 5) {
                Image(systemName: "mic.fill").foregroundStyle(Palette.coral)
                Text(remaining.map { "\($0)/\(DailyAllowance.freeLimit)" } ?? "∞")
                    .font(.game(15, relativeTo: .subheadline))
            }
            .foregroundStyle(Palette.ink)
            .padding(.horizontal, 10)
            .frame(height: 38)
            .background(Capsule().fill(Color.white))
            .overlay(Capsule().stroke(Palette.ink, lineWidth: 3))
            .accessibilityElement(children: .ignore)
            .accessibilityLabel(
                remaining.map { "マイクパワー 残り\($0)回" } ?? "マイクパワー 無制限"
            )

            Button(action: onOmikuji) {
                VStack(spacing: 0) {
                    Text("今日の").font(.game(9, relativeTo: .caption2))
                    Text("おみくじ").font(.game(12, relativeTo: .caption))
                }
                .foregroundStyle(.white)
                .frame(width: 58, height: 50)
                .background(Circle().fill(Palette.coral))
                .overlay(Circle().stroke(Palette.ink, lineWidth: 3))
            }
            .buttonStyle(.plain)
            .accessibilityLabel("今日のおみくじ")
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
    }
}

struct ProgressBar: View {
    let fraction: Double
    var fill: Color = Palette.teal

    var body: some View {
        GeometryReader { geometry in
            ZStack(alignment: .leading) {
                Capsule().fill(Color.white)
                Capsule()
                    .fill(fill)
                    .frame(width: max(0, min(1, fraction)) * geometry.size.width)
            }
            .overlay(Capsule().stroke(Palette.ink, lineWidth: 3))
        }
        .accessibilityHidden(true)
    }
}

private struct StageNode: View {
    let stage: Stage
    let state: StageNodeState
    let size: CGFloat
    let action: () -> Void

    @State private var pulse = false

    var body: some View {
        Button(action: action) {
            VStack(spacing: 4) {
                ZStack {
                    if case .current = state {
                        Circle()
                            .stroke(Color.white, lineWidth: 4)
                            .frame(width: size * 1.3, height: size * 1.3)
                            .scaleEffect(pulse ? 1.25 : 0.95)
                            .opacity(pulse ? 0 : 0.8)
                    }
                    Circle()
                        .fill(fillStyle)
                        .frame(width: size, height: size)
                        .overlay(Circle().stroke(Palette.ink, lineWidth: 3.5))
                        .shadow(color: edgeColor, radius: 0, x: 0, y: 4)
                    nodeContent
                }
                .frame(width: size * 1.3, height: size * 1.3)

                if case .cleared(let earned) = state {
                    StarsView(count: earned, size: 18)
                }
            }
        }
        .buttonStyle(.plain)
        .onAppear {
            withAnimation(.easeOut(duration: 1.6).repeatForever(autoreverses: false)) { pulse = true }
        }
        .accessibilityLabel(accessibilityText)
    }

    @ViewBuilder
    private var nodeContent: some View {
        switch state {
        case .cleared:
            Text("\(stage.number)").font(.game(size * 0.4, relativeTo: .title2)).foregroundStyle(Palette.ink)
        case .current:
            CreatureView(sound: stage.primarySound).frame(width: size * 0.85, height: size * 0.85)
        case .locked:
            Image(systemName: "lock.fill").font(.system(size: size * 0.36)).foregroundStyle(Palette.ink)
        case .proOnly:
            VStack(spacing: 0) {
                Image(systemName: "lock.fill").font(.system(size: size * 0.28))
                Text("Pro").font(.game(size * 0.22, relativeTo: .caption2))
            }
            .foregroundStyle(Palette.ink)
        }
    }

    private var fillStyle: LinearGradient {
        switch state {
        case .cleared:
            LinearGradient(colors: [Palette.sunLight, Palette.sun], startPoint: .top, endPoint: .bottom)
        case .current:
            LinearGradient(colors: [.white, Palette.paleSky], startPoint: .top, endPoint: .bottom)
        case .locked, .proOnly:
            LinearGradient(colors: [Palette.locked, Palette.lockedEdge.opacity(0.6)], startPoint: .top, endPoint: .bottom)
        }
    }

    private var edgeColor: Color {
        switch state {
        case .cleared: Palette.sunEdge
        case .current: Palette.ink
        case .locked, .proOnly: Palette.lockedEdge
        }
    }

    private var accessibilityText: String {
        let base = "ステージ\(stage.number) \(stage.title)"
        switch state {
        case .cleared(let earned): return "\(base)、クリア済み、星\(earned)つ"
        case .current: return "\(base)、挑戦できます"
        case .locked: return "\(base)、前のステージをクリアすると開きます"
        case .proOnly: return "\(base)、Proで利用できます"
        }
    }
}
