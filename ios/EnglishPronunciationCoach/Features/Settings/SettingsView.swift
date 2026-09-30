import StoreKit
import SwiftUI

struct SettingsView: View {
    @Environment(EntitlementStore.self) private var entitlements

    @State private var showsPaywall = false
    @State private var showsManageSubscriptions = false
    @State private var message: String?

    private var version: String {
        Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "-"
    }

    var body: some View {
        Form {
            Section("Pro") {
                if entitlements.isPro {
                    Label("Proをご利用中です", systemImage: "checkmark.seal.fill")
                } else {
                    Button("Proの内容を見る") { showsPaywall = true }
                    Text("Proでなくても、ステージ1は1日10回まで無料で練習できます。")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
                Button("購入を復元") { Task { await restore() } }
                Button("サブスクリプションを管理") { showsManageSubscriptions = true }
            }

            Section("プライバシー") {
                Text("発音の録音と分析は端末の中だけで行います。音声を外部のサーバーへ送ることはありません。アカウント登録は不要で、広告や追跡もありません。録音データは保存しません。")
                    .font(.footnote)
            }

            Section("情報") {
                Link("利用規約", destination: StoreConfig.termsURL)
                Link("プライバシーポリシー", destination: StoreConfig.privacyURL)
                Link("サポート", destination: StoreConfig.supportURL)
                LabeledContent("バージョン", value: version)
            }
        }
        .navigationTitle("設定")
        .manageSubscriptionsSheet(isPresented: $showsManageSubscriptions)
        .sheet(isPresented: $showsPaywall) { PaywallView() }
        .alert(
            "購入の復元",
            isPresented: Binding(get: { message != nil }, set: { if !$0 { message = nil } })
        ) {
            Button("OK") {}
        } message: {
            Text(message ?? "")
        }
    }

    private func restore() async {
        do {
            try await AppStore.sync()
            await entitlements.refresh()
            message = "購入情報を更新しました。"
        } catch {
            message = "購入を復元できませんでした。しばらくしてから、もう一度お試しください。"
        }
    }
}
