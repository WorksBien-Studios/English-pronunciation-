# TestFlight issue ledger

No TestFlight session has run for this app yet.

## Provisioning pending

- Status: `open`
- Stage: `preflight`
- Signature: `ios-source-credential-route-and-beta-group-missing`
- Resolved: Explicit bundle ID `com.worksbienstudios.englishpronunciationcoach` and App Store Connect app record `6817376615` are created.
- Symptom: The default branch does not yet contain the app's Xcode project or shared scheme, and the App Store Connect API credential route and beta-group resource ID are not configured.
- Prevention: The workflow is fail-closed and performs these checks on Linux before allocating macOS.
- Next action: Add the real iOS project, fill the Xcode and beta-group fields in the non-secret app map, configure the App Store Connect credential route, then switch the map state to `ready`.
