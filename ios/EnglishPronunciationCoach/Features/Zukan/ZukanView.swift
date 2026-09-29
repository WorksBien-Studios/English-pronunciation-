import SwiftData
import SwiftUI

/// 図鑑 tab: collected sound creatures. The grid adapts its column count to the available width.
struct ZukanView: View {
    @Query private var progress: [StageProgress]

    private let catalog = StageCatalog.shared

    private var unlocked: Set<Sound> {
        var result = Set<Sound>()
        for stage in catalog.stages {
            let earned = progress.first(where: { $0.stageID == stage.id })?.stars ?? 0
            if earned > 0 { result.formUnion(stage.sounds) }
        }
        return result
    }

    var body: some View {
        let collected = unlocked
        ScrollView {
            VStack(alignment: .leading, spacing: 14) {
                HStack {
                    Text("仲間を集めよう")
                        .font(.game(14, relativeTo: .subheadline))
                        .foregroundStyle(Palette.secondaryText)
                    Spacer()
                    Text("\(collected.count) / \(Sound.allCases.count)")
                        .font(.game(16, relativeTo: .headline))
                        .foregroundStyle(Palette.ink)
                }
                ProgressBar(fraction: Double(collected.count) / Double(Sound.allCases.count), fill: Palette.sun)
                    .frame(height: 14)

                LazyVGrid(columns: [GridItem(.adaptive(minimum: 130), spacing: 12)], spacing: 12) {
                    ForEach(Array(Sound.allCases.enumerated()), id: \.element.id) { index, sound in
                        ZukanCard(number: index + 1, sound: sound, isCollected: collected.contains(sound))
                    }
                }
                Text("ステージをクリアすると、図鑑に仲間が増えます。")
                    .font(.game(12, relativeTo: .caption))
                    .foregroundStyle(Palette.secondaryText)
            }
            .padding(16)
        }
        .background(SkyBackground())
        .navigationTitle("音の図鑑")
    }
}

private struct ZukanCard: View {
    let number: Int
    let sound: Sound
    let isCollected: Bool

    var body: some View {
        VStack(spacing: 2) {
            Text(String(format: "No.%02d", number))
                .font(.game(11, relativeTo: .caption2))
                .foregroundStyle(Palette.secondaryText)
                .frame(maxWidth: .infinity, alignment: .leading)
            CreatureView(sound: sound, isLocked: !isCollected)
                .frame(maxWidth: 96, maxHeight: 96)
            Text(isCollected ? sound.displayName : "？？？")
                .font(.game(15, relativeTo: .headline))
            Text(isCollected ? sound.ipa : "まだ出会っていない")
                .font(.game(12, relativeTo: .caption))
                .foregroundStyle(Palette.secondaryText)
        }
        .foregroundStyle(isCollected ? Palette.ink : Palette.secondaryText)
        .padding(10)
        .frame(maxWidth: .infinity)
        .background(
            RoundedRectangle(cornerRadius: 20, style: .continuous)
                .fill(isCollected ? Color.white : Color(hex: 0xF1F3FA))
        )
        .overlay(
            RoundedRectangle(cornerRadius: 20, style: .continuous)
                .stroke(Palette.ink.opacity(isCollected ? 1 : 0.4), style: StrokeStyle(lineWidth: 3, dash: isCollected ? [] : [6, 4]))
        )
        .accessibilityElement(children: .combine)
    }
}
