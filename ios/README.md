# English Pronunciation Coach — iOS app

SwiftUI app scaffold for the 音の島 concept in [`docs/game-ui-concept.md`](../docs/game-ui-concept.md),
built on the native iOS 18 adaptive shell from
[`lrodeveloperr/ios-18-shell`](https://github.com/lrodeveloperr/ios-18-shell) (pinned to a commit in the
generated Xcode project).

- iOS/iPadOS 18+, iPhone and iPad (regular-width iPad layouts use `.inspector` and adaptive grids)
- Bundle ID `com.worksbienstudios.englishpronunciationcoach`
- SwiftUI, SwiftData, StoreKit 2, AVFoundation; no third-party dependencies beyond the shell package

## Build

Open `ios/EnglishPronunciationCoach.xcodeproj` in Xcode 16+ and run the `EnglishPronunciationCoach` scheme.
The project file is **generated**; after adding, moving or removing source files run

```sh
python3 scripts/generate_xcodeproj.py
```

CI (`.github/workflows/ios.yml`) fails if the committed project is stale, then builds and tests on an
iPhone and an iPad simulator and does an unsigned Release build.

## Layout

```
ios/EnglishPronunciationCoach/
  App/            entry point and the shell-based root (冒険 / 図鑑 / 進捗 / 設定)
  Design/         theme, chunky buttons, outlined text, vector creatures, coach, mouth diagram
  Model/          Sound, Stage/StageCatalog (JSON), star rating, daily allowance, streak
  EngineBridge/   the seam to the on-device pronunciation engine (protocol + typed results)
  Audio/          microphone capture (memory only) and model-audio playback (speech synthesis)
  Persistence/    SwiftData models (stage progress, daily usage, error-pattern counts)
  Store/          StoreKit 2 entitlement + configuration
  Features/       Map, Stage, Challenge, Clear, Zukan, Omikuji, Progress, Settings, Paywall
  Resources/      stages.json (lesson content), PrivacyInfo.xcprivacy, Info.plist additions, assets
ios/EnglishPronunciationCoachTests/   unit tests for the pure logic and the content file
```

## Decisions and known gaps

- **Engine is not linked yet.** `EngineFactory` returns a preview engine in Debug (so the UI flow can be
  exercised) and `UnavailablePronunciationEngine` in Release, which always answers "retry". The app never
  fabricates a pronunciation score. Integrating the C++ core/ONNX model behind `PronunciationEngine` is the next
  engineering step, and the pre-UI engine gate in the product spec still applies to shipping.
- **Subscription group ID is a placeholder** (`StoreConfig.subscriptionGroupID`). Until it is set the paywall
  shows a notice. Product IDs match the listing pack. Prices are always read from StoreKit.
- **Swift language mode is 5** with the Swift 6 toolchain. The spec targets Swift 6 mode; move over after the
  first green CI run and a concurrency audit of `AudioRecorder`/`EntitlementStore`.
- **Art is mockup-grade** (vector, drawn in code from the design mockup). Replace with final illustrator art.
- **Content is a starter set** (4 stages, 6 sounds). Only stage 1 is free; the free/Pro split follows the listing pack.
- **No sound effects, Live Activities, or Game Center.** Haptics use `.sensoryFeedback`.
- **Privacy:** audio stays in memory and is discarded after analysis; no network calls, no analytics.
