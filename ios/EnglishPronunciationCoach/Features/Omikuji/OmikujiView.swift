import SwiftUI

/// A free once-a-day fortune with a short pronunciation tip. No purchase, no randomness for sale.
struct OmikujiView: View {
    @Environment(\.dismiss) private var dismiss
    @AppStorage("omikujiDay") private var drawnDay = ""

    private struct Fortune {
        let rank: String
        let tip: String
    }

    private let fortunes: [Fortune] = [
        Fortune(rank: "大吉", tip: "Rは舌先をつけないのがラッキー！"),
        Fortune(rank: "中吉", tip: "Lは舌先を上あごにしっかりつけよう。"),
        Fortune(rank: "小吉", tip: "THは舌を歯の間に。声は出さなくてOK。"),
        Fortune(rank: "吉", tip: "Vは上の歯を下唇に軽く当てよう。"),
        Fortune(rank: "末吉", tip: "語尾の子音は、母音を足さずに止めよう。")
    ]

    private var todayKey: String { DailyAllowance.dayKey(.now) }
    private var hasDrawnToday: Bool { drawnDay == todayKey }

    /// Stable per calendar day (does not use Swift's per-launch hash seed).
    private var todayFortune: Fortune {
        let sum = todayKey.unicodeScalars.reduce(0) { $0 + Int($1.value) }
        return fortunes[sum % fortunes.count]
    }

    var body: some View {
        NavigationStack {
            VStack(spacing: 20) {
                if hasDrawnToday {
                    slip
                } else {
                    VStack(spacing: 12) {
                        CoachView(mood: .cheer).frame(width: 120, height: 120)
                        Text("1日1回、無料でひけます。")
                            .font(.game(15, relativeTo: .body))
                            .foregroundStyle(Palette.ink)
                    }
                    Button("おみくじをひく") { drawnDay = todayKey }
                        .buttonStyle(.chunky)
                        .padding(.horizontal, 24)
                }
            }
            .padding(.vertical, 24)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(SkyBackground())
            .navigationTitle("今日のおみくじ")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("とじる") { dismiss() }
                }
            }
        }
    }

    private var slip: some View {
        VStack(spacing: 16) {
            VStack(spacing: 10) {
                Text("発音みくじ")
                    .font(.game(14, relativeTo: .subheadline))
                    .foregroundStyle(Color(hex: 0x9E1F16))
                Text(todayFortune.rank)
                    .font(.game(72, relativeTo: .largeTitle))
                    .foregroundStyle(Palette.coral)
                VStack(spacing: 4) {
                    Text("きょうのひとこと")
                        .font(.game(12, relativeTo: .caption))
                        .foregroundStyle(Color(hex: 0x7A4B00))
                    Text(todayFortune.tip)
                        .font(.game(16, relativeTo: .body))
                        .multilineTextAlignment(.center)
                        .foregroundStyle(Palette.ink)
                }
                .padding(14)
                .frame(maxWidth: .infinity)
                .background(RoundedRectangle(cornerRadius: 14).fill(Color(hex: 0xFFF3D6)))
            }
            .padding(20)
            .frame(maxWidth: 360)
            .gameCard(radius: 14)
            Text("1日1回・無料・課金はありません")
                .font(.game(12, relativeTo: .caption))
                .foregroundStyle(Palette.ink)
        }
        .padding(.horizontal, 24)
    }
}
