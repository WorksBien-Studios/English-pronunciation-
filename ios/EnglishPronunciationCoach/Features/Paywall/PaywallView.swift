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
                        description: Text("購読の設定が完了すると、ここから購入できます。")
                    )
                }
            }
            .toolbar {
                ToolbarItem(placement: .topBarTrailing) {
                    Button {
                        dismiss()
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .symbolRenderingMode(.hierarchical)
                            .foregroundStyle(.secondary)
                    }
                    .accessibilityLabel("閉じる")
                }
            }
        }
    }

    private var marketing: some View {
        VStack(spacing: 16) {
            Text("r")
                .font(.system(size: 40, weight: .bold, design: .serif))
                .foregroundStyle(.white)
                .frame(width: 72, height: 72)
                .background(Color.brand, in: RoundedRectangle(cornerRadius: 17, style: .continuous))
                .accessibilityHidden(true)
            Text("Proで、すべてのステージを")
                .font(.title.bold())
                .multilineTextAlignment(.center)
            Text("無料の1日10回をこえて、じっくり練習できます。")
                .font(.subheadline)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
            VStack(alignment: .leading, spacing: 12) {
                Label("発音分析の回数制限をなくす", systemImage: "infinity")
                Label("すべてのステージを練習（TH、BとV、F）", systemImage: "flag.fill")
            }
            .font(.subheadline)
            .padding(16)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(Color(.secondarySystemGroupedBackground), in: RoundedRectangle(cornerRadius: 12, style: .continuous))
            Text("Proでなくても、ステージ1「Rの森」は1日10回まで無料で練習できます。結果が返らなかった録音は回数に数えません。")
                .font(.footnote)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
        }
        .padding()
    }
}
