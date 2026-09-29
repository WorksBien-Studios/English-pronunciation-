# TestFlight issue ledger

No TestFlight session has run for this app yet.

## Provisioning pending

- Status: `open`
- Stage: `preflight`
- Signature: `app-record-and-ios-source-missing`
- Symptom: The App Store Connect record cannot be completed until the explicit bundle ID is registered, and the default branch does not yet contain the app's Xcode project or shared scheme.
- Prevention: The workflow is fail-closed and performs these checks on Linux before allocating macOS.
- Next action: Register `com.worksbienstudios.englishpronunciationcoach`, create the minimal App Store Connect app record, add the real iOS project, fill the non-secret app map, and verify the repository credential route.
