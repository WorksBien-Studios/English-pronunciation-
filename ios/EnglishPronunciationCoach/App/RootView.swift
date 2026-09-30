import SwiftUI
import iOS18Shell

/// Native iOS 18 tab bar (iPhone) / sidebar-adaptable tabs (iPad) from the shell package.
/// Every screen uses system components only; layouts adapt to the width they are given.
struct RootView: View {
    @StateObject private var navigator = AppShellNavigator()

    private var adventure: AppTab {
        AppTab(id: "adventure", title: "冒険", systemImage: "flag.fill") { AdventureView() }
    }

    private var zukan: AppTab {
        AppTab(id: "zukan", title: "図鑑", systemImage: "book.closed.fill") { ZukanView() }
    }

    private var progress: AppTab {
        AppTab(id: "progress", title: "進捗", systemImage: "chart.bar.fill") { ProgressTabView() }
    }

    private var settings: AppTab {
        AppTab(id: "settings", title: "設定", systemImage: "gearshape.fill") { SettingsView() }
    }

    var body: some View {
        AppShellView(
            tabIDs: ["adventure", "zukan", "progress", "settings"],
            navigator: navigator
        ) {
            appShellTab(adventure, navigator: navigator)
            appShellTab(zukan, navigator: navigator)
            appShellTab(progress, navigator: navigator)
            appShellTab(settings, navigator: navigator)
        }
        .tint(Color.brand)
        .environment(\.locale, Locale(identifier: "ja_JP"))
    }
}
