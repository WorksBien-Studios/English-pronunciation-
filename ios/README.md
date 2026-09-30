# English Pronunciation Coach — iOS app

SwiftUI app with a native-only iOS 18 UI ([`docs/native-ui.md`](../docs/native-ui.md)),
built on the native iOS 18 adaptive shell from
[`lrodeveloperr/ios-18-shell`](https://github.com/lrodeveloperr/ios-18-shell) (pinned to a commit in the
generated Xcode project).

- iOS/iPadOS 18+, iPhone and iPad (one column below 900 pt of container width, list + detail at 900 pt and above)
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
  Design/         tint colour, width-adaptive layout helper, stars, cue list, live waveform
  Model/          Sound, Stage/StageCatalog (JSON), star rating, daily allowance, streak
  EngineBridge/   the seam to the on-device pronunciation engine (protocol + typed results)
  Audio/          microphone capture (memory only) and model-audio playback (speech synthesis)
  Persistence/    SwiftData models (stage progress, daily usage, error-pattern counts)
  Store/          StoreKit 2 entitlement + configuration
  Features/       Adventure (list + stage detail), Challenge, Clear, Zukan, Progress, Settings, Paywall
  Resources/      stages.json (lesson content), PrivacyInfo.xcprivacy, Info.plist additions, assets
ios/EnglishPronunciationCoachTests/   unit tests for the pure logic and the content file
```

## Decisions and known gaps

- **The production engine is linked.** `EngineFactory` uses the actor-isolated C++/ONNX adapter in Debug and
  Release. CI fetches the locked model, verifies its SHA-256, bundles the complete engine content pack, and checks
  both resources in the built app. Audio-quality feedback remains available without loading the model. Latency,
  memory, offline behaviour and native-speaker accuracy on a physical iPhone are verified by the developer's own
  hands-on testing.
- **Subscription group ID is a placeholder** (`StoreConfig.subscriptionGroupID`). Until it is set the paywall
  shows a notice. Product IDs match the listing pack. Prices are always read from StoreKit.
- **Swift 6 language mode** (`SWIFT_VERSION = 6.0`, strict concurrency). `AudioRecorder`, `EntitlementStore` and the
  engine adapter are actor-isolated; the realtime audio tap uses a `Sendable` buffer.
- **No custom art.** The UI is system components and SF Symbols only.
- **Content is a starter set** (4 stages, 6 sounds). Only stage 1 is free; the free/Pro split follows the listing pack.
- **No sound effects, Live Activities, or Game Center.** Haptics use `.sensoryFeedback`.
- **Privacy:** audio stays in memory and is discarded after analysis; no network calls, no analytics.
