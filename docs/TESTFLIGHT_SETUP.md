# English Pronunciation Coach — TestFlight wiring

Status: `PROVISIONING_PENDING`

The minimal App Store Connect record, explicit bundle ID, and internal TestFlight group now exist. No App Store listing copy has been entered. The repository contains a fail-closed TestFlight pipeline. It performs cheap Linux preflight before allocating a paid macOS runner, archives once, exports once, uploads the same IPA once, then uses a cheap Linux job to deliver the exact processed build to the mapped beta group and attach it to the matching editable App Store version. It never submits the version for App Review.

## Locked identity

- Repository: `lrodeveloperr/English-pronunciation-`
- App Store Connect app ID: `6817376615`
- Bundle ID: `com.worksbienstudios.englishpronunciationcoach`
- Apple team: `49SQ3XQ68Q`
- Internal beta group: `Internal QA`
- Beta group resource ID: `5199a6ce-ee5a-4f2d-91ce-992dbe15e3bf`
- Signing style: automatic
- Export method: App Store Connect
- Distribution certificate: Apple Distribution
- Platform: `IOS`
- Marketing version: `1.0`
- Primary App Store language: Japanese
- SKU: `WB-EN-PRON-JP-IOS-001`

## Deliberate blockers

The workflow cannot allocate macOS until `.github/testflight-app-map.json` has `state: ready` and contains:

1. the exact non-secret API key ID and issuer ID;
2. the committed Xcode project/workspace path and type;
3. the shared app scheme.

The release workflow is now locked to automatic signing with Apple Distribution export; it does not use ad-hoc signing or mutable profile names. The iOS app source is not yet present on the default branch. The completed pronunciation engine currently remains on its development branch. Do not point the map at the generic shell example; map the actual English Pronunciation Coach project after the UI repository decision is final.

## GitHub Actions credentials

Configure these repository secrets without exposing their values:

- `ASC_KEY_ID`
- `ASC_ISSUER_ID`
- `ASC_PRIVATE_KEY`

Configure these repository variables with the same non-secret identifiers:

- `ASC_EXPECTED_KEY_ID`
- `ASC_EXPECTED_ISSUER_ID`

The workflow compares the secret route, repository variables and app map before macOS allocation. It also authenticates once and verifies the exact App Store Connect app ID and bundle ID.

## First run

The first accepted upload must use the full lane. Dispatch **English Pronunciation Coach TestFlight** with:

- confirmation: `UPLOAD ENGLISH PRONUNCIATION TESTFLIGHT`
- `validated_sha`: the full immutable commit SHA already carrying the successful `English Engine Stress` check and the final iOS project.

After the first build is processed, tester-delivered and listing-attached, record the successful run as the baseline before adding an express lane. Until then, no express claim is valid.

## Review-artifact invariant

The `Internal QA` group contains both designated tester accounts (2 testers; 0 builds at provisioning). The release lane archives, signs, exports, and uploads exactly once, then attaches that same Apple-processed build to the matching editable App Store version. App Review must use that attached build without rebuilding or re-signing. This proves build readiness only; listing metadata, screenshots, privacy, compliance, agreements, and review information remain separate submission gates.
