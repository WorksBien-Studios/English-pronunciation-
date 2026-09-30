# Native UI

**Status:** implemented (2026-09-30). Replaces the creature-based 音の島 concept in
[`game-ui-concept.md`](game-ui-concept.md). Mockup: [`design/native-ui/mockup.html`](../design/native-ui/mockup.html).

The app UI uses Apple system components only: `TabView` (via the ios-18-shell package), `NavigationStack`, `List`,
`Gauge`, `ProgressView`, Swift Charts, `Canvas`, SF Symbols, system colours and materials, Dynamic Type, and StoreKit's
`SubscriptionStoreView`. There are no third-party UI dependencies and no custom illustration.

## Layout rules

| Layout | When | Structure |
|---|---|---|
| One column | container width < 900 pt (iPhone, iPad mini, iPad in Split View) | Bottom tab bar on iPhone, top tabs on iPad. Content is limited to a 620 pt readable column. |
| Two columns | container width ≥ 900 pt (iPad Pro 11" landscape, 13") | Top tabs, a 340 pt list on the left, detail on the right. |

The switch is made from the space the view receives (`WidthAdaptive`, `Layout.twoColumnMinWidth`), never from the
device model.

## Screens

冒険 (today card + stage list, stage detail) · 録音チャレンジ (word, live waveform, result per word) · ステージクリア ·
図鑑 (grid + detail) · 進捗 (streak, weekly chart, difficult-sound history) · 設定 · Pro paywall.

## Differences from the mockup

The mockup shows a few things the engine bridge does not expose yet. They were **not** invented in the app:

- Per-phoneme result chips and the pitch/energy chart in 結果. `PronunciationResult` carries only
  `intelligible`, `targetSoundProduced`, `likelySubstitution` and `confidence`; showing more needs the bridge to
  return alignment and prosody data first.
- Stars in the per-word result. Stars are a stage-level rating and appear on the clear screen and in the lists.
- The quest list (お手本を聞く / 口の形をまねる). The stage detail shows the words with model audio and the cues.
- The tongue-position diagram was removed on purpose (asset liability). Articulation is explained in text.
- The おみくじ feature was removed.

## Third-party components considered

None are used. If the native waveform needs more polish, `DSWaveformImage` (MIT) is the one dependency to consider.
Confetti/particle packages (ConfettiSwiftUI, Vortex; both MIT) are optional for the clear screen. Licences were read
from each repository's `LICENSE` file on 2026-09-30.

## Japanese copy

Copy follows `game-ui-concept.md` §6b: です・ます, no spaces between kana, Latin letters and Japanese, encouraging
tone and no negative words for the learner. It has not had a native-speaker review (owner decision).
