# App Store Listing Pack — 英語発音コーチ

**Status:** `DRAFT_READY`  
**Storefront:** Japan  
**Platform:** iOS / iPadOS 18+  
**Primary locale:** Japanese (`ja-JP`)  
**Release:** Version 1.0, new app  
**Research and Apple-source review date:** 2026-09-29  
**Canonical product specification:** `docs/product-specification.md`

This is the complete launch-copy and App Store Connect decision pack. It is not a submission authorization. Build-dependent controls are explicitly marked pending.

> **Revision 2 (2026-09-29, owner decision):** the initial assessment and weakness map are deferred beyond release 1.0. All copy, screenshots and review notes below describe only what the 1.0 build does: a stage map, per-word recording with sound-level Japanese feedback and retry, stars and a creature collection, a progress tab, and a free Stage 1 with ten valid recordings a day. Promises the binary does not keep were removed: assessment, minimal-pair/sentence practice, own-recording playback, review lists, accent/rhythm/linking practice and vowel or final-consonant modules.

## 1. Listing decision

### Locked positioning

> 日本人が苦手な英語の音を見つけ、なぜ違うのか、舌・唇・息・声をどう使えばよいかまで日本語で示す、オフライン発音コーチ。

The listing deliberately avoids the crowded generic promises “AI英会話,” “ネイティブのように,” and “必ず上達.” It leads with the evidenced gap: sound-level feedback plus actionable Japanese correction.

### Locked metadata

| Field | Final Japanese copy | Limit | Count |
|---|---|---:|---:|
| Name | **英語発音コーチ｜日本人向け発音矯正・練習** | 30 characters | 20 |
| Subtitle | **RとL・THなどの発音を、直し方まで日本語で練習** | 30 characters | 24 |
| Promotional text | **点数だけで終わらない発音練習。どの音が違って聞こえたかを示し、舌・唇・息・声の使い方を日本語で説明。端末内で分析し、1日10回まで無料で試せます。** | 170 characters | 73 |
| Keywords | `カタカナ英語,発音記号,スピーキング,子音` | 100 UTF-8 bytes | 57 bytes |

Revision 2 (owner decision to defer the assessment): removed `アクセント`, `リンキング` and `母音` because the 1.0 build does not teach accent, linking or vowel contrasts (a 2.3.7 relevance risk); the field now holds only terms the build supports. Keyword-field changes in the earlier 2026-09-29 copy review: removed `音素練習` (not a real search term), `リスニング` (the app does not teach listening comprehension, a 2.3.7 relevance risk), `RL` (does not match how learners type the pair), `イントネーション` (not supported by any listed feature) and `TH` (already in the subtitle, so it only wasted bytes); added `カタカナ英語`, `発音記号`, `リンキング`, `子音` and `母音`, each tied to a feature the description states. Web spot-checks on 2026-09-29 (no Apple keyword-popularity or Japan search-volume data was available) showed `発音記号`, `スピーキング` and `アクセント` in live competitor titles, and `カタカナ英語` used by learners to describe the problem this app solves. Treat the field as a hypothesis and review it against App Store Connect search-term analytics after launch.

Do not add competitor names, `AI英会話`, `TOEIC`, `英検`, `IELTS`, `無料`, or `オフライン` to the keyword field merely to chase traffic. The first four are either misleading for this narrow product or unsupported at launch; the latter terms belong in readable copy when truthful.

### ASO and web-SEO status — 2026-09-29

The indexed App Store fields now cover the primary intent (`英語発音`), correction intent (`発音矯正`), practice intent (`練習`), Japanese-speaker focus, sound-level correction differentiation, and supported secondary vocabulary without duplicating title or subtitle terms in the keyword field. The added `練習` term is supported by the high-traction `英語発音トレーニング` lane already recorded below. This is the strongest evidence-backed launch hypothesis available without private Apple Search Popularity data; validate it after launch using App Store Connect search impressions and conversion data.

Search-result conversion ASO remains pending until the finished icon and authentic screenshots are checked at small/search-result size. Public-web SEO for the marketing URL is a separate HTML audit—page title, meta description, headings, canonical URL, indexability and structured data are not attested by this App Store listing file.

### Name status: `LOCKED`

The full title is descriptive, truthful, within Apple's 30-character limit, and was not found as an exact App Store title during the 2026-09-28 check. This is a naming collision screen, not trademark clearance.

| Candidate | Decision | Reason |
|---|---|---|
| 英語発音コーチ｜日本人向け発音矯正・練習 | **Selected** | Captures `英語発音`, `コーチ`, `発音矯正`, and `練習`; immediately identifies the Japanese-speaker focus while covering the validated practice intent. |
| 英語発音矯正｜日本語コーチ | Rejected | Shorter, but “日本語コーチ” can imply Japanese-language tutoring. |
| 日本人の英語発音トレーナー | Rejected | Clear but loses the stronger correction intent `発音矯正`. |
| 発音フィックス | Rejected | Brand-like, lower search pull, and requires explanation. |

## 2. Live lane evidence

Current Japan App Store listings and visible reviews were checked on 2026-09-28. Rating counts are demand proxies, not download estimates.

| App | Live Japan signal | What the listing/reviews prove | Positioning implication |
|---|---:|---|---|
| [ELSA Speak](https://apps.apple.com/jp/app/id1083804886) | 4.6, 63k ratings | Large willingness to use pronunciation feedback; visible complaints include bugs, purchase restoration, confusing entitlement changes, strict recognition and support friction. | Keep pricing and entitlements stable, restore purchases, and never make an account a condition of purchase access. |
| [英語発音トレーニング](https://apps.apple.com/jp/app/id1497634724) | 4.6, 9,940 ratings | A simple five-minute listen-record-result loop attracts substantial use. | Keep the first session fast and understandable; avoid broad-course clutter. |
| [BoldVoice](https://apps.apple.com/jp/app/id1567841142) | 4.7, 2,179 ratings | Users praise phoneme-first progressive practice. Recent reviews report English-only explanations, assessment bugs, language switching and rejection of legitimate non-American variants. | Lead with Japanese correction, stable onboarding and conservative accepted variants. |
| [発音博士](https://apps.apple.com/jp/app/id885771578) | 3.6, 759 ratings | Users value IPA and self/reference comparison. A 2025 review asks for slow reference playback; other reviews describe severe or surprising scoring without instruction. | Include slow model pronunciation, familiar IPA, and physical correction rather than a naked score. |
| [Speakometer](https://apps.apple.com/jp/app/id1529508890) | 4.7, 321 ratings | Confirms demand for IPA, accent options and personalized feedback, but its App Store language is English despite Japanese metadata. | Make Japanese UI and explanations native, consistent and explicit. |
| Pronouncepro / Echoia / generic AI pronunciation entrants | Insufficient ratings for a summary | Recent supply uses broad feature piles, trials, ads or data collection, but has not yet shown comparable Japanese rating traction. | Win through focus, privacy, transparent daily free use and corrective quality—not feature count. |

### Review-derived promises the listing may make

- Japanese explanations for what to change physically.
- Sound-level feedback in Japanese and progressive, stage-based practice.
- Normal and slow model pronunciation.
- IPA display.
- Conservative feedback that can return `retry` when evidence is weak.
- On-device analysis, no account, no advertising and a clear daily free allowance—only if the release build preserves these facts.

### Claims the listing must not make

- “100% accurate,” “most accurate,” “clinically validated,” or “guaranteed improvement.”
- “Native-speaker recordings”; launch audio is Apple system model pronunciation.
- British-English coaching or support for every world accent unless accepted variants and lessons are implemented and tested.
- Open-ended speech analysis, conversation practice, exam preparation or 65,000-word search.
- “AI” as a headline merely for discoverability. The value is correction, not the implementation label.

## 3. Final Japanese description

Copy exactly from this block after confirming build parity and deploying the legal URLs.

```text
英語の発音を採点されても、
「何が違うのか」「どう直せばよいのか」が分からなければ、同じ間違いを繰り返してしまいます。

英語発音コーチは、日本語話者のための、英語発音の矯正練習アプリです。

録音した声を確認し、目標の音を出せたか、近く聞こえた別の音の候補、直し方を日本語で示します。舌・唇・息・声の使い方を確かめながら、単語ごとに練習できます。

【練習の流れ】

・RとL、TH、BとV、Fの音を、ステージ形式で集中練習
・目標の音を出せたかと、近く聞こえた音の候補を表示
・お手本音声（iOS標準の音声合成）を通常速度とゆっくり再生で確認、IPA発音記号に対応
・ステージをクリアしてスターや図鑑を集め、毎日の練習を継続
・繰り返し見つかった苦手な音を記録し、進捗タブで確認

【日本語だから、直し方が分かる】

点数だけを表示して終わりません。

「舌先をどこに置くか」
「唇をどう動かすか」
「息を止めるか流すか」
「声を振動させるか」

など、次の一回で試せる具体的な修正方法を日本語で説明します。

【「伝わるか」と「音が合っているか」を分けて確認】

・相手に伝わる可能性
・目標の音を出せた可能性

を分けて確認できます。分析の確信度が低い場合は、無理に正誤を断定せず、録音環境の確認や再試行をご案内します。

【短く、毎日続けやすく】

聞く、話す、直し方を確認する、もう一度試す。
約5分の短い練習を中心に、余分な画面操作を減らして発音に集中できるよう設計しています。

【プライバシーを守る端末内分析】

発音の録音と分析は端末内で処理されます。音声を分析のために外部サーバーへ送信しません。アカウント登録は不要で、広告や追跡もありません。録音データは初期設定では保存しません。発音練習と分析はオフラインでも利用できます。

【無料で使える内容】

・ステージ1「Rの森」（RとL）のすべての練習
・1日10回の有効な発音分析
・お手本音声、IPA、日本語解説

無音、音割れ、録音中断などで結果が返らなかった場合は、無料回数を消費しません。未使用回数の翌日への繰り越しはありません。

【Proでできること】

・発音分析の回数制限を解除
・すべてのステージ（TH、BとV、F）の練習

Proは月額または年額の自動更新サブスクリプションです。購入確定時にApple IDアカウントに請求されます。購入前にApp Storeが地域に応じた料金と更新期間を表示します。自動更新は現在の期間終了の24時間前までに解約しない限り継続します。管理・解約はApple IDのサブスクリプション設定から行えます。購入の復元にも対応します。

利用規約：
https://www.apple.com/legal/internet-services/itunes/dev/stdeula/

プライバシーポリシー：
https://worksbienstudios.com/apps/english-pronunciation-coach/privacy
```

## 4. Screenshot production brief

Create three authentic Japanese screenshots for both iPhone and iPad. Use the same captions and sample learner profile so the page tells one coherent story.

| Position | Role | Caption | Required real screen | Free/paid clarity |
|---:|---|---|---|---|
| 1 | Path | **ゲーム感覚で、苦手な音を練習** | Stage map (冒険 tab) showing Stage 1 available and later stages marked Pro; on iPad the two-column layout with the stage detail. | Stage 1 is free; later stages carry a visible `Pro` label. |
| 2 | Path | **直し方が、日本語でわかる** | Correction screen showing IPA, target/likely substitution and one concise articulatory instruction. | Use the free Stage 1 (R/L) module. |
| 3 | Proof | **発音を練習して、すぐ再チェック** | Retest screen showing listen → record → feedback → retry, with intelligibility and target sound separated. | Use a free-module example; do not imply every module is free. |

Production rules:

- use current app UI only—no conceptual mockup presented as the product;
- portrait iPhone master at an accepted 6.9-inch size, preferably 1320 × 2868;
- portrait iPad master at 2064 × 2752, captured in the **regular-width two-column NavigationSplitView state**; do not use the compact/single-column iPhone-like presentation for the iPad store set;
- JPEG or PNG with no alpha channel;
- large Japanese caption readable at search-result size;
- no competitor names, platform logos, hardware frames copied without rights, prices or unverifiable scores;
- use fictional sample data consistently across all frames;
- if a visible screen is Pro-only, add a small clear `Pro` label near the relevant feature;
- keep microphone permission dialogs and paywalls out of the three launch frames.

### Screenshot shell masters

The editable launch shell masters live in `assets/screenshots/shells/` and cover **both supported device classes**. The rounded slot is replaced with an authentic capture from the matching device during screenshot production; do not put iPhone captures into the iPad shell or vice versa.

**iPhone — 1320 × 2868**
- `01-map.svg` — **ゲーム感覚で、苦手な音を練習**
- `02-correction.svg` — **直し方が、日本語でわかる**
- `03-progress.svg` — **発音を練習して、すぐ再チェック**

**iPad — 2064 × 2752**
- `ipad-01-map.svg` — **ゲーム感覚で、苦手な音を練習**
- `ipad-02-correction.svg` — **直し方が、日本語でわかる**
- `ipad-03-progress.svg` — **発音を練習して、すぐ再チェック**

`shell-spec.json` contains deterministic canvas sizes, replacement-slot coordinates, and capture-layout requirements for both device classes. The iPad set must visibly demonstrate the app's native two-column layout.

### Icon brief

A single speech shape containing two clearly separated sound-wave strokes, suggesting “hear the difference” rather than generic conversation. Use a calm indigo/teal palette on a solid background, no text, flags, AI sparkles, score numbers or photorealistic mouth. Verify at 60 × 60 and alongside the live `英語 発音` search grid before locking.

## 5. App Store Connect field map

| Field | Entry / decision | Status |
|---|---|---|
| Primary language | Japanese | Locked |
| App UI language | Japanese; English appears only as lesson material | Locked; verify binary |
| Primary category | Education | Locked |
| Secondary category | None at launch | Locked |
| Availability | Japan only at launch | Recommended; confirm before creating app record |
| Price | Free download | Locked |
| In-app purchases | Auto-renewable Pro monthly and annual, one subscription group | Locked |
| Introductory offer | None at launch | Locked; renewable free tier is the trial |
| Advertising | None | Locked |
| Account | Not required | Locked |
| Required hardware | Microphone-equipped supported iPhone/iPad only | Locked |
| Minimum OS | iOS/iPadOS 18 | Locked |
| Age rating | Expected 4+; answer every live questionnaire item truthfully | Provisional until App Store Connect questionnaire |
| Copyright | `2026 WorksBien Studios Inc.` | Proposed; confirm account/legal presentation |
| Bundle ID | `com.worksbienstudios.englishpronunciationcoach` | Proposed; reserve before implementation |
| SKU | `WB-EN-PRON-JP-IOS-001` | Proposed; immutable after app record creation |
| Support URL | `https://worksbienstudios.com/apps/english-pronunciation-coach/support/` | **Live and verified 2026-09-28** |
| Marketing URL | `https://worksbienstudios.com/apps/english-pronunciation-coach/` | **Live and verified 2026-09-28** |
| Privacy URL | `https://worksbienstudios.com/apps/english-pronunciation-coach/privacy/` | **Live and verified 2026-09-28** |
| Terms | Apple Standard EULA | Locked |
| Release method | Manual release after approval | Recommended |
| App preview video | Omit at launch | Locked; optional post-launch |

Additional public legal pages:

- Terms: `https://worksbienstudios.com/apps/english-pronunciation-coach/terms/`
- Subscription conditions: `https://worksbienstudios.com/apps/english-pronunciation-coach/subscriptions/`
- Specified Commercial Transactions Act disclosure: `https://worksbienstudios.com/apps/english-pronunciation-coach/commercial-transactions/`

## 6. Subscription metadata

Proposed identifiers must be checked for availability before they become final.

| Item | Monthly | Annual |
|---|---|---|
| Product ID | `com.worksbienstudios.englishpronunciationcoach.pro.monthly` | `com.worksbienstudios.englishpronunciationcoach.pro.annual` |
| Reference name | English Pronunciation Coach Pro Monthly | English Pronunciation Coach Pro Annual |
| Japanese display name | Pro（月額） | Pro（年額） |
| Japanese description | 発音分析の回数制限を解除し、すべてのステージを利用 | 発音分析の回数制限を解除し、すべてのステージを利用 |
| Duration | 1 month | 1 year |
| Japan launch price | ¥600 | ¥4,800 |
| App Store promotion | Do not promote separately at launch | Do not promote separately at launch |

Subscription group display name: **英語発音コーチ Pro**.

The in-app paywall must load localized price and period from StoreKit, disclose renewal, show Privacy Policy and Terms, and provide Restore Purchases and Manage Subscription. Never hard-code yen prices into reusable UI strings.

## 7. App Privacy and permissions answer pack

These answers are conditional on the final binary containing no analytics, advertising, crash-reporting or network SDK that changes them.

| Question | Planned answer | Evidence required before submission |
|---|---|---|
| Does the developer or a third party collect data? | **No, we do not collect data from this app.** | Final network inspection, SDK inventory and privacy manifest. StoreKit transaction handling must remain device/App Store based without developer collection. |
| Tracking / ATT | **No tracking; ATT not requested.** | No IDFA access or tracking SDK. |
| Audio data | **Not collected.** Microphone audio is processed locally and raw recordings are discarded unless the user explicitly saves one locally. | Trace audio buffers and persistence paths in release build. |
| Diagnostics | **Not collected by the developer.** | Confirm no third-party crash/analytics SDK and review Apple's platform diagnostics setting separately. |
| Account data | **None.** | No login, email, name or custom user ID. |
| Purchases | **Not collected by the developer.** Entitlement is read through StoreKit. | No server receipts, analytics or custom purchase database. |
| Privacy choices URL | Omit | No developer-collected data requiring a choice portal. |

Microphone purpose string (`NSMicrophoneUsageDescription`):

> 発音を録音し、端末上で音ごとのフィードバックを表示するためにマイクを使用します。録音は分析のために外部サーバーへ送信されません。

Request microphone access only when the learner first records—not at launch.

## 8. Content, legal and technical declarations

| App Store Connect question | Draft answer | Rationale / evidence |
|---|---|---|
| Third-party content rights | **Yes — rights secured.** | The bundled Wav2Vec2 checkpoint is Apache-2.0 and ONNX Runtime is MIT. Include notices. Apple system voices are accessed through public APIs; no Apple voice recording is redistributed. All lessons, Japanese copy and graphics must be original or separately licensed. |
| Export compliance | Set `ITSAppUsesNonExemptEncryption` to `NO`, assuming the final app contains no custom/non-exempt cryptography. | Reconfirm from final dependency inventory. Standard Apple commerce/system security alone does not create a custom encryption feature. |
| Advertising identifier | No | No advertising or tracking SDK. |
| User-generated content | No | Users do not publish or share recordings inside the app. |
| Health/medical claims | No | Pronunciation education only; never describe speech therapy, diagnosis or treatment. |
| Kids Category | No | General education app; do not use “For Kids” metadata. |
| Regulated medical device | No | Not a medical product. |
| Sign-in with Apple | Not applicable | No account or third-party sign-in. |
| Location, camera, contacts, photos | Not requested | Microphone is the only core permission. |
| Notifications | Off by default; request only after an explicit practice-reminder action if implemented | Do not request during onboarding. |

## 9. Expected age-rating answers

Expected result: **4+**, subject to Apple's live questionnaire.

Answer `None` for violence, sexual content/nudity, profanity or crude humor, horror/fear themes, alcohol/tobacco/drugs, gambling, loot boxes, unrestricted web access, contests and medical/treatment information. The app contains no user-generated public content, messaging or advertising. Answer microphone/voice recording questions exactly as presented; voice recording alone does not justify raising the content rating.

## 10. App Review notes

Use English for operational clarity:

```text
This is a Japanese-language, offline English-pronunciation training app for Japanese speakers. No account is required.

Core review path:
1. Launch the app. The stage map (冒険 tab) opens with Stage 1 “Rの森” available; later stages are marked Pro.
2. Open Stage 1 and start a recording. Grant microphone access when prompted.
3. Say the shown word. The result screen gives sound-level feedback in Japanese (whether the target sound was produced, a likely substituted sound, and one articulation tip) and offers a retry.
4. Continue through the stage's words to earn stars; a cleared stage unlocks the next one.

Free access:
- Stage 1 (R/L) is free.
- Ten valid scored recordings are available per local calendar day.
- Silence, clipping, interrupted recordings, permission failures, and internal scoring failures do not consume the allowance.

Pro access:
- Open Settings > Pro, or tap a locked stage.
- Monthly and annual auto-renewable subscriptions unlock unlimited scoring and all stages.
- Restore Purchases and Manage Subscription are available from Settings > Pro.

Audio and privacy:
- Microphone audio is analyzed on device and is not uploaded for pronunciation analysis.
- Raw recordings are not retained by default.
- The app contains no ads, tracking, login, or custom backend.
- Model pronunciation is generated on device with AVSpeechSynthesizer. It is not represented as a human recording and is not used as the scoring reference waveform.

The app must be tested on a physical microphone-equipped device. Pronunciation analysis works in airplane mode. StoreKit purchase actions require the normal App Store sandbox environment.
```

Reviewer contact must be populated with the current App Store Connect contact immediately before submission. Do not commit a personal phone number to the public repository.

## 11. Applicable App Review controls

Current Apple sources reviewed on 2026-09-29:

- [App Review Guidelines](https://developer.apple.com/app-store/review/guidelines/)
- [App information fields](https://developer.apple.com/help/app-store-connect/reference/app-information/app-information/)
- [Platform version information](https://developer.apple.com/help/app-store-connect/reference/app-information/platform-version-information)
- [App privacy](https://developer.apple.com/help/app-store-connect/manage-app-information/manage-app-privacy)
- [Screenshots](https://developer.apple.com/help/app-store-connect/reference/app-information/screenshot-specifications/)
- [Subscriptions](https://developer.apple.com/app-store/subscriptions/)

| Guideline | Application | Required release evidence |
|---|---|---|
| 1.5 Developer Information | Support contact must be reachable. | **Pass for the website layer:** the app-specific support page is live with the current public email; reconfirm immediately before submission. |
| 1.6 Data Security | Voice and learning history remain protected locally. | Storage review, file protection and no unexpected network transfer. |
| 2.1 App Completeness | The engine, URLs and both subscriptions must be functional. | Final build/device test and IAP review assets. |
| 2.3 Accurate Metadata | Every privacy, offline, free-limit and feature claim must match the binary. | Build/listing parity audit. |
| 2.3.2 Paid Content | Screenshots and description must not imply all modules/unlimited scoring are free. | Page-level paid-boundary review. |
| 2.3.3 Screenshots | Images must show the app in use. | Authentic current iPhone/iPad UI captures. |
| 2.3.7 Metadata | Name, subtitle and keywords must remain relevant and trademark-clean. | Final App Store Connect validation. |
| 2.4.1 Hardware Compatibility | iPhone and iPad layouts must function on the declared devices. | Device/simulator matrix plus physical iPhone audio test. |
| 2.4.2 Resource Use | The 197 MB model must not cause excessive memory, heat or battery use. | Physical iPhone SE 2020 latency/memory/thermal test. |
| 3.1.1 In-App Purchase | Digital Pro features use StoreKit subscriptions. | Products attached to submission; transaction verification and lifecycle tests. |
| 3.1.2 Subscriptions | Ongoing value, duration, renewal and full price must be clear. | Paywall, Terms, Privacy, Restore and Manage Subscription screenshots/tests. |
| 4.0 Design | The app must provide lasting educational utility, not a thin TTS wrapper. | Lessons, sound-level feedback, progress history and practice workflow. |
| 4.2 Minimum Functionality | The custom on-device scoring and correction engine is the core value. | Working engine, not static reference content alone. |
| 5.1.1 Data Collection and Storage | Microphone permission and local-only data claims must be accurate. | Purpose string, privacy manifest, SDK/network inventory. |
| 5.2 Intellectual Property | Model/runtime licences and original lesson rights must be documented. | Apache-2.0/MIT notices and content-rights ledger. |

## 12. Submission checklist

### App Information

- [ ] Create/reserve the final bundle ID and SKU.
- [ ] Confirm the full app name is accepted in App Store Connect.
- [ ] Set primary language to Japanese and category to Education.
- [ ] Complete the live age-rating questionnaire; expected result 4+.
- [ ] Answer third-party content rights `Yes` and retain the model/runtime licence evidence.
- [ ] Confirm Japan-only availability and manual release.
- [ ] Enter live Privacy Policy and Support URLs.
- [ ] Use `2026 WorksBien Studios Inc.` only if it matches the rights holder/account presentation.

### Version metadata and assets

- [ ] Paste the locked name, subtitle, promotional text, description and 96-byte keyword field.
- [ ] Capture three authentic Japanese iPhone screenshots at an accepted 6.9-inch size.
- [ ] Capture three authentic Japanese iPad screenshots at 2064 × 2752 or another current accepted 13-inch size.
- [ ] Verify no alpha channel, incorrect crop, debug data, personal data or unsupported claim.
- [ ] Verify the icon at small size and against the relevant live search grid.
- [ ] Omit the preview video at launch.

### Build, privacy and permissions

- [ ] Complete the physical iPhone SE 2020 performance gate.
- [ ] Verify every launch exercise and the complete developer pronunciation pass.
- [ ] Inspect final network traffic in normal, offline, purchase and restore flows.
- [ ] Confirm no analytics, ads, tracking, crash SDK or remote inference exists.
- [ ] Inspect `PrivacyInfo.xcprivacy`, required-reason APIs and all SDK signatures.
- [ ] Confirm microphone permission is requested just in time with the locked Japanese purpose string.
- [ ] Verify raw audio deletion/default retention behavior and local file protection.
- [ ] Test VoiceOver, Dynamic Type, contrast, reduced motion and minimum tap targets.

### StoreKit

- [ ] Create one subscription group with monthly and annual products.
- [ ] Enter Japanese subscription names/descriptions and attach review screenshots.
- [ ] Set Japan prices to ¥600/month and ¥4,800/year; confirm the live StoreKit display.
- [ ] Submit the first subscriptions with the app version under Apple's live workflow.
- [ ] Test purchase, cancellation, pending, failed, offline, restore, auto-renew-off, grace, billing retry, expiry, refund, revoke and unverified transaction states.
- [ ] Confirm locked content never opens from an unverified or expired entitlement.
- [ ] Confirm free daily use resets correctly and failures never consume an attempt.

### Legal and review

- [x] Deploy and open Privacy, Terms, Subscription, 特商法, Support and Marketing pages from a signed-out browser (verified 2026-09-28).
- [ ] Add Apache-2.0 and MIT notices in-app and in the repository.
- [ ] Confirm the final archive contains only assets with documented commercial rights.
- [ ] Add current reviewer contact privately in App Store Connect.
- [ ] Paste the review notes and verify every stated navigation path.
- [ ] Attach the final build and both subscriptions to the same submission.
- [ ] Perform a final build/listing/privacy/paywall parity check immediately before submission.

## 13. Current blockers and final decision

**Decision: `DRAFT_READY`.** Metadata, positioning, screenshot captions and the answer pack are coherent and within the current field limits.

The following remain build-stage blockers, not drafting failures:

1. final bundle ID/SKU and App Store Connect record;
2. final archive, SDK/privacy-manifest inspection and physical-device evidence;
3. authentic screenshots and finished icon;
4. created StoreKit products, localized pricing and review assets;
5. live App Store Connect validation of the final metadata.

Do not label the app ready to submit until those five items are evidenced.
