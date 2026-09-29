import StoreKit
import SwiftUI

/// Pro paywall built on StoreKit's `SubscriptionStoreView`: prices, periods, renewal terms,
/// Restore and policy links come from StoreKit, so no price is ever hard-coded.
struct PaywallView: View {
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Group {
                if StoreConfig.isSubscriptionGroupConfigured {
                    SubscriptionStoreView(groupID: StoreConfig.subscriptionGroupID) {
                        marketing
                    }
                    .storeButton(.visible, for: .restorePurchases)
                    .subscriptionStorePolicyDestination(url: StoreConfig.termsURL, for: .termsOfService)
                    .subscriptionStorePolicyDestination(url: StoreConfig.privacyURL, for: .privacyPolicy)
                } else {
                    ContentUnavailableView(
                        "Proは準備中です",
                        systemImage: "hourglass",
                        description: Text("サブスクリプションの設定が完了すると、ここから購入できます。")
                    )
                }
            }
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button("あとで") { dismiss() }
                }
            }
        }
    }

    private var marketing: some View {
        VStack(spacing: 14) {
            CoachView(mood: .cheer).frame(width: 96, height: 96)
            OutlinedText(text: "Proで冒険を広げよう", size: 22, fill: .white)
            VStack(alignment: .leading, spacing: 10) {
                benefit("発音分析の回数制限をなくす", symbol: "infinity")
                benefit("すべてのステージ・発音レッスン", symbol: "flag.fill")
                benefit("苦手な音の履歴と復習リスト", symbol: "chart.bar.fill")
                benefit("アクセント・リズム・リンキングの練習", symbol: "waveform")
            }
            Text("Proでなくても、1日10回まで無料でチャレンジできます。")
                .font(.footnote)
                .foregroundStyle(.secondary)
        }
        .padding()
    }

    private func benefit(_ text: String, symbol: String) -> some View {
        Label(text, systemImage: symbol)
            .font(.game(15, relativeTo: .body))
            .foregroundStyle(Palette.ink)
    }
}
