# TestFlight issue ledger

No TestFlight session has run for this app yet.

## Provisioning pending

- Status: `open`
- Stage: `preflight`
- Signature: `ios-source-and-credential-route-missing`
- Resolved: Explicit bundle ID `com.worksbienstudios.englishpronunciationcoach`, App Store Connect app record `6817376615`, and internal beta group `Internal QA` (`5199a6ce-ee5a-4f2d-91ce-992dbe15e3bf`) are created and mapped.
- Symptom: The default branch does not yet contain the app's Xcode project or shared scheme, and the App Store Connect API credential route is not configured.
- Prevention: The workflow is fail-closed and performs these checks on Linux before allocating macOS.
- Next action: Add the real iOS project, fill the Xcode fields in the non-secret app map, configure the App Store Connect credential route, then switch the map state to `ready`.

## Tester target

- Status: `resolved`
- `Internal QA` contains both designated internal tester accounts.
- The delivery helper assigns the exact processed build to that group and attaches the same build to the editable App Store version for later review without re-signing.
