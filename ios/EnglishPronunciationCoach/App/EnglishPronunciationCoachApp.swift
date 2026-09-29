import SwiftData
import SwiftUI

@main
struct EnglishPronunciationCoachApp: App {
    @State private var entitlements = EntitlementStore()
    private let container = AppPersistence.makeContainer()

    var body: some Scene {
        WindowGroup {
            RootView()
                .environment(entitlements)
                .task { entitlements.start() }
        }
        .modelContainer(container)
    }
}
