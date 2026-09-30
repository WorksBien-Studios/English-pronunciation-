import SwiftUI

/// Shown after the last word: stars, the sound added to the 図鑑, and what each star means.
struct ClearView: View {
    let stage: Stage
    let summary: ClearSummary
    let onDone: () -> Void
    let onRetry: () -> Void

    var body: some View {
        NavigationStack {
            ScrollView {
                VStack(spacing: 20) {
                    VStack(spacing: 10) {
                        Text(summary.stars > 0 ? "ステージクリア" : "あと少しでクリア")
                            .font(.largeTitle.bold())
                        StarsView(count: summary.stars, size: 44)
                        Text(stage.title)
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                    }
                    .padding(.top, 24)

                    if let sound = summary.newSound {
                        newSoundCard(sound)
                    }
                    breakdownCard
                }
                .padding(16)
                .readableColumn()
            }
            .background(Color(.systemGroupedBackground))
            .safeAreaInset(edge: .bottom) {
                VStack(spacing: 10) {
                    Button(action: onDone) {
                        Text("次へ").frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .controlSize(.large)
                    if summary.stars < 3 {
                        Button(action: onRetry) {
                            Text("★3をめざして、もう一度").frame(maxWidth: .infinity)
                        }
                        .buttonStyle(.bordered)
                        .controlSize(.large)
                    }
                }
                .padding(.horizontal, 16)
                .padding(.vertical, 10)
                .frame(maxWidth: Layout.readableWidth + 32)
                .frame(maxWidth: .infinity)
                .background(.bar)
            }
            .navigationBarTitleDisplayMode(.inline)
        }
        .sensoryFeedback(.success, trigger: summary.id)
    }

    private func newSoundCard(_ sound: Sound) -> some View {
        HStack(spacing: 16) {
            Text(sound.symbol)
                .font(.system(size: 44, weight: .semibold, design: .serif))
                .foregroundStyle(Color.brand)
                .frame(width: 72, height: 72)
                .background(Color.brand.opacity(0.12), in: RoundedRectangle(cornerRadius: 16, style: .continuous))
            VStack(alignment: .leading, spacing: 2) {
                Text("図鑑に加わりました").font(.footnote).foregroundStyle(.secondary)
                Text("\(sound.label) \(sound.ipa)").font(.title3.bold())
                Text(sound.japaneseNote).font(.footnote).foregroundStyle(.secondary)
            }
            Spacer(minLength: 0)
        }
        .card()
        .accessibilityElement(children: .combine)
    }

    private var breakdownCard: some View {
        VStack(spacing: 0) {
            row("相手に伝わりそう", achieved: summary.outcome.intelligible, stars: "★1")
            Divider()
            row("\(stage.primarySound.ipa) の音が出せた", achieved: summary.outcome.targetSoundProduced, stars: "★2")
            Divider()
            row("くり返しても安定している", achieved: summary.outcome.stable, stars: "★3")
        }
        .padding(.horizontal, 16)
        .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
    }

    private func row(_ title: String, achieved: Bool, stars: String) -> some View {
        HStack(spacing: 10) {
            Image(systemName: achieved ? "checkmark.circle.fill" : "circle.dashed")
                .font(.title3)
                .foregroundStyle(achieved ? Color.green : Color.secondary)
            Text(title).font(.subheadline)
            Spacer()
            Text(stars).font(.caption).foregroundStyle(.secondary)
        }
        .padding(.vertical, 12)
        .accessibilityElement(children: .combine)
    }
}
