import SwiftData
import SwiftUI

/// 図鑑 tab: every launch sound as a card. A sound is added when its stage is cleared.
/// Wide windows show the grid beside the selected sound; narrow ones push the detail.
struct ZukanView: View {
    @Query private var progress: [StageProgress]

    private enum Filter: String, CaseIterable, Identifiable {
        case all = "すべて"
        case collected = "集めた音"
        case remaining = "まだの音"
        var id: String { rawValue }
    }

    @State private var filter = Filter.all
    @State private var selected: Sound = .r
    @State private var pushed: Sound?

    private let catalog = StageCatalog.shared

    private func stage(of sound: Sound) -> Stage? {
        catalog.stages.first { $0.sounds.contains(sound) }
    }

    private func stars(of stage: Stage?) -> Int {
        guard let stage else { return 0 }
        return progress.first { $0.stageID == stage.id }?.stars ?? 0
    }

    private func isCollected(_ sound: Sound) -> Bool {
        stars(of: stage(of: sound)) > 0
    }

    private var collectedCount: Int {
        Sound.allCases.filter(isCollected).count
    }

    private var shown: [Sound] {
        Sound.allCases.filter { sound in
            switch filter {
            case .all: true
            case .collected: isCollected(sound)
            case .remaining: !isCollected(sound)
            }
        }
    }

    var body: some View {
        WidthAdaptive {
            HStack(spacing: 0) {
                ScrollView { grid(wide: true).padding(16) }
                    .frame(width: 460)
                Divider()
                SoundDetailView(sound: selected, stage: stage(of: selected), isCollected: isCollected(selected))
                    .id(selected)
            }
        } compact: {
            ScrollView { grid(wide: false).padding(16).readableColumn() }
        }
        .background(Color(.systemGroupedBackground))
        .navigationTitle("図鑑")
        .navigationDestination(item: $pushed) { sound in
            SoundDetailView(sound: sound, stage: stage(of: sound), isCollected: isCollected(sound))
        }
    }

    private func grid(wide: Bool) -> some View {
        VStack(alignment: .leading, spacing: 16) {
            VStack(alignment: .leading, spacing: 8) {
                HStack {
                    Text("集めた音").font(.headline)
                    Spacer()
                    Text("\(collectedCount) / \(Sound.allCases.count)")
                        .font(.body.monospacedDigit())
                        .foregroundStyle(.secondary)
                }
                ProgressView(value: Double(collectedCount), total: Double(Sound.allCases.count))
                Text("ステージをクリアすると、その音が図鑑に加わります。")
                    .font(.caption)
                    .foregroundStyle(.secondary)
            }
            .card()

            Picker("表示", selection: $filter) {
                ForEach(Filter.allCases) { Text($0.rawValue).tag($0) }
            }
            .pickerStyle(.segmented)

            LazyVGrid(columns: [GridItem(.adaptive(minimum: 140), spacing: 12)], spacing: 12) {
                ForEach(shown) { sound in
                    Button {
                        if wide { selected = sound } else { pushed = sound }
                    } label: {
                        SoundCard(
                            number: (Sound.allCases.firstIndex(of: sound) ?? 0) + 1,
                            sound: sound,
                            stage: stage(of: sound),
                            stars: stars(of: stage(of: sound)),
                            isSelected: wide && sound == selected
                        )
                    }
                    .buttonStyle(.plain)
                }
            }
        }
    }
}

private struct SoundCard: View {
    let number: Int
    let sound: Sound
    let stage: Stage?
    let stars: Int
    let isSelected: Bool

    private var collected: Bool { stars > 0 }

    private var example: PracticeWord? {
        stage?.words.first { $0.target == sound }
    }

    var body: some View {
        VStack(spacing: 4) {
            Text(String(format: "No.%02d", number))
                .font(.caption2.monospacedDigit())
                .foregroundStyle(.secondary)
            Text(sound.symbol)
                .font(.system(size: 44, weight: .semibold, design: .serif))
                .foregroundStyle(collected ? Color.brand : Color.secondary.opacity(0.4))
            Text(sound.label).font(.subheadline.weight(.semibold))
            if let example {
                Text("\(example.text) \(example.ipa)")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .minimumScaleFactor(0.8)
            }
            if collected {
                StarsView(count: stars, size: 13)
            } else if let stage {
                Label("ステージ\(stage.number)で解放", systemImage: "lock.fill")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .labelStyle(.titleAndIcon)
            }
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 14)
        .padding(.horizontal, 8)
        .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 14, style: .continuous))
        .overlay {
            RoundedRectangle(cornerRadius: 14, style: .continuous)
                .stroke(isSelected ? Color.brand : Color.clear, lineWidth: 2)
        }
        .accessibilityElement(children: .ignore)
        .accessibilityLabel(
            collected
                ? "\(sound.label)、\(sound.ipa)、集めた音、星\(stars)つ"
                : "\(sound.label)、\(sound.ipa)、まだ集めていない音"
        )
    }
}
