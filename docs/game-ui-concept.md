# UI Concept — 音の島 (Sound Islands)

**Status:** `LOCKED` (design concept) — not an implementation authorization  
**Decided:** 2026-09-29  
**Applies to:** iPhone and iPad, iOS/iPadOS 18+, Japanese UI  
**Mockup source:** [`design/ui-mockup/`](../design/ui-mockup/) (Design-canvas `.dc.html` artboards + `canvas.json`)

This document locks the visual and interaction concept for the app. It does **not** change the product behaviour, scoring engine, App Store name/subtitle, pricing or privacy claims defined in [`product-specification.md`](product-specification.md) and [`app-store-listing-ja-JP.md`](app-store-listing-ja-JP.md). Where those files conflict with this one on behaviour, they win.

## 1. What appeals to Japanese users (design premises)

| Premise | How the concept answers it |
|---|---|
| Collecting and completing sets (図鑑, character cards) | Every hard English sound is an original creature; clearing its stage adds it to the 音の図鑑. |
| Cute, personable mascots (ゆるキャラ style) | A guide character (コーチ) plus one creature per sound; round shapes, blush cheeks, expressive faces. |
| Clear progression (stage maps, ★1–3, Lv/EXP) | Islands map with numbered stages, star ratings, level and EXP. |
| Small daily rituals | Daily 今日のおみくじ (free, no purchase) and a stamp/streak view. |
| Bright, dense, glossy game UI | Chunky outlined buttons, ribbons, sunbursts, rounded gothic type, seigaiha (wave) pattern in the sea. |
| Trust and clarity for an education purchase | Free daily allowance and Pro boundary are always shown plainly; price text always comes from StoreKit. |

These are design hypotheses, not evidence of market fit. Validate with Japanese-language TestFlight users before treating them as proven.

## 2. Structure

- **Tabs:** 冒険 (map) · 図鑑 · 進捗 · 設定, via the shell's `TabView`/`Tab` with `.sidebarAdaptable` (bottom bar on iPhone, top bar/sidebar on iPad).
- **Map:** islands of numbered stages. Each stage is one lesson from the content model (`Lesson`, `JapaneseErrorPattern`, `MinimalPair` …). Stage 3 (Rの森, /r/ ↔ /l/) is the reference stage in the mockup.
- **Stage detail:** creature pair, mouth cross-section (English tongue position solid, Japanese ラ行 dotted), tongue/lips/breath/voice cues, three quests: お手本を聞く → 口の形をまねる → 録音チャレンジ, then a 仕上げテスト (retest).
- **Challenge:** target word, IPA, creature coach, live waveform, obvious recording state, tap-to-stop.
- **Clear screen:** stars, new creature, plain-language breakdown, EXP.
- **Star meaning (fixed, maps to the spec's scoring philosophy):** ★1 = the word would likely be understood; ★2 = the target sound was produced; ★3 = stable on a repeat. Stars never reward a single arbitrary "accent score".
- **図鑑:** collected creatures with No., name and IPA; locked entries show a silhouette and the stage that unlocks them.
- **おみくじ:** once-daily free fortune containing a short pronunciation tip.
- **Free/Pro:** the 10 valid analyses per day appear as マイクパワー 7/10. Paywall lists exactly the Pro benefits in the listing pack.

**Creature names** are generic sound labels (Rくん, Lくん, THくん, Vくん, Fくん, Bくん); the guide is コーチ. This avoids brand-name collisions (see [`trademark-search-worksheet.md`](trademark-search-worksheet.md)).

## 3. Screens in the mockup

iPhone: 冒険マップ · ステージ詳細 · 録音チャレンジ · ステージクリア · 音の図鑑 · 今日のおみくじ · Pro（購読）  
iPad: 冒険マップ＋インスペクタ · 録音チャレンジ（2ペイン）· 図鑑＋詳細 · 縦向きステージ詳細 · Pro（購読シート）  
Parts: 音のなかま, コーチ, 口の断面図, iPhone タブバー, iPad タブバー, キャラクターシート.

## 4. iPad is a first-class layout

- Regular width: map + `.inspector` for the selected stage; challenge as two panes; 図鑑 as an adaptive grid + detail; portrait stage as a two-column layout.
- Compact width (iPhone, iPad Split View at one-third): single column, iPhone arrangement.
- Layout must respond to size classes and container width (`ViewThatFits`, adaptive `LazyVGrid`, `containerRelativeFrame`), never to device model.
- Minimum 44 pt hit targets and Dynamic Type support on every control.

## 5. Native Apple implementation map

| Need | Apple-native tool |
|---|---|
| Navigation shell | `ios-18-shell` (`TabView`/`Tab`, `.sidebarAdaptable`, per-tab `NavigationStack`), `NavigationSplitView`, `.inspector` |
| Art (creatures, map, diagram, stamps) | SwiftUI `Path`/`Shape`/`Canvas` (vector, resolution independent) |
| Backgrounds | `MeshGradient` (iOS 18), gradients |
| Motion | `PhaseAnimator`, `KeyframeAnimator`, `TimelineView` (waveform from the `AVAudioEngine` level meter) |
| Confetti / clear burst | SpriteKit via `SpriteView` |
| Feedback | `.sensoryFeedback` haptics |
| Type | Hiragino Maru Gothic (system font on iOS); outlined display text via layered `Text` |
| Meters/progress | `Gauge`, `ProgressView`, Swift Charts |
| Model audio | `AVSpeechSynthesizer` (labelled as iOS speech synthesis) |
| Mic permission | `AVAudioApplication.requestRecordPermission`, requested at the first recording |
| Paywall | StoreKit `SubscriptionStoreView` with custom marketing content; prices and periods come from StoreKit |
| Persistence | SwiftData (per spec) |

No third-party UI, animation or game libraries are required.

## 6. Guardrails (compliance and product)

- **No loot boxes/gacha or purchasable randomness.** おみくじ is free and cosmetic (guideline 3.1.1).
- **No penalties for retrying.** No lives, hearts or timers. Wording is encouraging (e.g. おしい！あと少し); it must never shame an accent.
- **Honest scoring.** Low-confidence audio returns `retry`, never a fabricated star (spec: conservative feedback).
- **Prices and terms:** never hard-code yen amounts; show StoreKit price/period, auto-renew and cancellation text, Terms and Privacy links, Restore, and an obvious close control (guideline 3.1.2). The free allowance is stated on the paywall.
- **Model audio** is labelled as iOS speech synthesis, not a native-speaker recording.
- **No social feed, leaderboards or accounts** (spec V1 exclusions). Game Center is not used.
- **Age rating:** re-answer the questionnaire against the final build; cartoon creatures with no violence are expected to stay 4+.
- **Assets:** all characters, patterns and diagrams in the mockup are original. Do not copy an existing game's or pronunciation app's creature, UI or diagram. A web-based similarity screen was done on 2026-09-29 (see [`character-similarity-screen.md`](character-similarity-screen.md)); it led to design and name changes but is not legal clearance. A formal trademark search is still required before the assets ship.

## 6a. Character expression rules (Japanese conventions)

Reviewed 2026-09-29 against common Japanese character-design and emoticon conventions.

- **Resting mouth is closed and tiny** (a small "ω" smile). A permanently open mouth reads as 驚き (shock) or ぽかん (blank), so no creature keeps an open mouth by default.
- **Open or shaped mouths are a signal, not a default.** Three states exist: `rest` (default), `show` (the articulation cue, e.g. /r/ rounded lips, /l/ tongue tip up, /θ/ tongue between teeth; used only on cue screens such as the challenge hint) and `joy` (small open smile, celebration screens only). In the app, `PhaseAnimator` swaps `rest` → `show` when the hint appears and back afterwards.
- **Charm points instead of open mouths at rest:** /θ/ has a small side-tongue てへぺろ, /v/ a single 八重歯 (yaeba, a small fang, widely considered cute in Japan), /b/ puffed cheeks. Tongue-out is playful, not rude, only in this cheeky てへぺろ form.
- **Coach:** `happy` = ^ ^ eyes with a closed smile; `cheer` = "> <" eyes with a small open smile (the ≧∀≦ joy pattern; do not pair "> <" eyes with a flat or wavy mouth, which reads as pain, ＞＜); `wow` is not used in shipped screens.
- Never show a downturned or frowning mouth on any character, including on `retry` or low-score feedback.
- **Eyes:** solid dark glossy eyes with two highlights (きらきら), set low and wide with pink cheeks (baby-schema proportions). White-scleral round eyes were rejected because they read as staring or startled.
- **Joy face:** closed "^ ^" eyes plus a small open smile, the strongest joy signal in Japanese emoticons and character art.
- **てへぺろ:** the /θ/ creature winks with a small side tongue; without the wink the tongue reads as drooling.
- **Distinct silhouettes:** each creature has its own top feature so the 図鑑 entries are recognisable as shadows: /r/ curl, /l/ antenna with a bulb, /θ/ round ears, /v/ pointed ears, /f/ leaf, /b/ bubbles.

## 6b. Writing rules for all customer-visible Japanese

- Kanji-kana mixed adult register; no word-spacing (分かち書き) between kana, and no spaces between Latin letters/numbers and Japanese (Rの森, Proを始める, 1日10回).
- UI labels are neutral (です/ます or noun phrases); characters may speak casually (〜よ, 〜だよ).
- Store and legal wording follows Apple's Japanese conventions (e.g. 購入確定時にApple IDアカウントに請求されます).
- Prefer 単語 to 言葉 when referring to the practice word; 仕上げテスト (not しあげ), 仲間 for creatures in running text.

## 7. Impact on other documents (follow-ups, not done here)

- `app-store-listing-ja-JP.md` screenshot brief and captions assume the earlier weakness-map UI; rewrite the three captions/frames for the map, stage and clear screens once real UI exists (screenshots must show the real app, guideline 2.3.3).
- The icon brief (speech bubble + two waves) still fits the コーチ character.
- `product-specification.md` places skinning after the engine's pre-UI gate. This concept does not waive that gate; the SwiftUI build must wait for it or be explicitly re-scoped by the owner.

## 8. Open items

1. Final names and polished art for the mascot and creatures (illustrator pass; current SVG art is a mockup).
2. Formal trademark/design search for the final names and art (the mockup screen is done; see the similarity screen).
3. Japanese-language TestFlight validation of tone, difficulty and reward pacing.
4. Decide the launch creature count (mockup shows 6) and stage count against the launch curriculum.
5. Sound design (optional): short native `AVAudioPlayer` effects for tap, clear and stamp; respect the silent switch.
