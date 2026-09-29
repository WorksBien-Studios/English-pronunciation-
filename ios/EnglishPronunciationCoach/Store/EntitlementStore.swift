import Foundation
import Observation
import StoreKit

/// Reads Pro entitlement from StoreKit 2. Only verified, non-revoked transactions count;
/// no server, no receipts sent anywhere.
@MainActor
@Observable
final class EntitlementStore {
    private(set) var isPro = false

    @ObservationIgnored private var updatesTask: Task<Void, Never>?

    func start() {
        guard updatesTask == nil else { return }
        updatesTask = Task { [weak self] in
            await self?.refresh()
            for await update in Transaction.updates {
                if case .verified(let transaction) = update {
                    await transaction.finish()
                }
                await self?.refresh()
            }
        }
    }

    func refresh() async {
        var pro = false
        for await result in Transaction.currentEntitlements {
            guard case .verified(let transaction) = result else { continue }
            if StoreConfig.productIDs.contains(transaction.productID), transaction.revocationDate == nil {
                pro = true
            }
        }
        isPro = pro
    }
}
