import Foundation

enum StoreConfig {
    static let monthlyProductID = "com.worksbienstudios.englishpronunciationcoach.pro.monthly"
    static let annualProductID = "com.worksbienstudios.englishpronunciationcoach.pro.annual"
    static var productIDs: Set<String> { [monthlyProductID, annualProductID] }

    /// Replace with the subscription group ID from App Store Connect
    /// (group display name: 英語発音コーチ Pro). Until then the paywall shows a notice.
    static let subscriptionGroupID = "REPLACE_WITH_SUBSCRIPTION_GROUP_ID"
    static var isSubscriptionGroupConfigured: Bool { !subscriptionGroupID.hasPrefix("REPLACE") }

    static let termsURL = URL(string: "https://www.apple.com/legal/internet-services/itunes/dev/stdeula/")!
    static let privacyURL = URL(string: "https://worksbienstudios.com/apps/english-pronunciation-coach/privacy/")!
    static let supportURL = URL(string: "https://worksbienstudios.com/apps/english-pronunciation-coach/support/")!
}
