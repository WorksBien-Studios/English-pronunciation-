import SwiftUI

/// Stage-clear celebration: stars, the collected creature, and a plain breakdown of what was shown.
struct ClearView: View {
    let stage: Stage
    let summary: ClearSummary
    let onDone: () -> Void
    let onRetry: () -> Void

    var body: some View {
        ZStack {
            SunburstBackground()
            ScrollView {
                VStack(spacing: 18) {
                    OutlinedText(
                        text: summary.stars > 0 ? "ステージクリア！" : "もう少しでクリア！",
                        size: 28,
                        fill: .white
                    )
                    .padding(.top, 24)

                    StarsView(count: summary.stars, size: 64)

                    if let sound = summary.newSound {
                        newFriendCard(sound)
                    }
                    breakdownCard
                }
                .padding(.horizontal, 20)
            }
        }
        .safeAreaInset(edge: .bottom) {
            VStack(spacing: 10) {
                Button("次のステージへ", action: onDone)
                    .buttonStyle(.chunky)
                if summary.stars < 3 {
                    Button("もう一度★3をめざす", action: onRetry)
                        .buttonStyle(ChunkyButtonStyle(tone: .secondary))
                }
            }
            .padding(.horizontal, 20)
            .padding(.vertical, 10)
            .background(.ultraThinMaterial)
        }
        .sensoryFeedback(.success, trigger: summary.id)
    }

    private func newFriendCard(_ sound: Sound) -> some View {
        HStack(spacing: 8) {
            CreatureView(sound: sound, mouth: .joy)
                .frame(width: 120, height: 120)
                .overlay(alignment: .topLeading) {
                    Text("NEW!")
                        .font(.game(13, relativeTo: .caption))
                        .foregroundStyle(.white)
                        .padding(.horizontal, 9)
                        .padding(.vertical, 2)
                        .background(Capsule().fill(Palette.coral))
                        .overlay(Capsule().stroke(Palette.ink, lineWidth: 3))
                        .rotationEffect(.degrees(-10))
                }
            VStack(alignment: .leading, spacing: 4) {
                Text(sound.ipa).font(.game(12, relativeTo: .caption)).foregroundStyle(Palette.secondaryText)
                Text(sound.displayName).font(.game(24, relativeTo: .title2))
                Text("\(sound.ipa)の音の仲間ができたよ！").font(.game(14, relativeTo: .subheadline))
            }
            .foregroundStyle(Palette.ink)
            Spacer(minLength: 0)
        }
        .padding(14)
        .gameCard(radius: 28)
    }

    private var breakdownCard: some View {
        VStack(spacing: 0) {
            breakdownRow("相手に伝わりそう", achieved: summary.outcome.intelligible, stars: "★1")
            Divider()
            breakdownRow("\(stage.primarySound.ipa)の音が出せた", achieved: summary.outcome.targetSoundProduced, stars: "★2")
            Divider()
            breakdownRow("もう一度できたら★3", achieved: summary.outcome.stable, stars: "★3")
        }
        .gameCard(radius: 24)
    }

    private func breakdownRow(_ title: String, achieved: Bool, stars: String) -> some View {
        HStack(spacing: 10) {
            Image(systemName: achieved ? "checkmark.circle.fill" : "circle.dashed")
                .font(.title3)
                .foregroundStyle(achieved ? Palette.teal : Palette.secondaryText)
            Text(title).font(.game(15, relativeTo: .subheadline))
            Spacer()
            Text(stars).font(.game(12, relativeTo: .caption)).foregroundStyle(Palette.secondaryText)
        }
        .foregroundStyle(Palette.ink)
        .padding(.horizontal, 16)
        .padding(.vertical, 12)
        .accessibilityElement(children: .combine)
    }
}

/// Radiating sun-ray backdrop.
struct SunburstBackground: View {
    var body: some View {
        Canvas { context, size in
            var ctx = context
            let center = CGPoint(x: size.width / 2, y: size.height * 0.26)
            let radius = max(size.width, size.height) * 1.4
            let rays = 18
            Art.fill(Path(CGRect(origin: .zero, size: size)), Color(hex: 0xFFF3B0), &ctx)
            for index in 0..<rays where index % 2 == 0 {
                let start = Angle.degrees(Double(index) * 360 / Double(rays))
                let end = Angle.degrees(Double(index + 1) * 360 / Double(rays))
                var wedge = Path()
                wedge.move(to: center)
                wedge.addArc(center: center, radius: radius, startAngle: start, endAngle: end, clockwise: false)
                wedge.closeSubpath()
                Art.fill(wedge, Color(hex: 0xFFE27A), &ctx)
            }
        }
        .ignoresSafeArea()
        .accessibilityHidden(true)
    }
}
