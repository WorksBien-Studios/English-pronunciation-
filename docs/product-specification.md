# Japan iOS Opportunity — Japanese-Specific English Pronunciation Coach

**Status:** BUILD  
**Market:** Japan iOS App Store  
**Product type:** Education / pronunciation training  
**Locked App Store name:** 英語発音コーチ｜日本人のための発音矯正  
**Locked subtitle:** RとL・THなど苦手な音を診断、直し方まで日本語で  
**Locked launch price:** Free download + Pro at ¥600/month or ¥4,800/year  
**Research date:** 2026-09-28

## Decision

Build a narrowly focused English-pronunciation correction app for native Japanese speakers.

Do **not** build a general English tutor, conversation chatbot, vocabulary app, or translated clone of ELSA/BoldVoice.

The product wedge is:

> Global pronunciation products have sophisticated diagnosis but weaker Japanese explanations.  
> Japanese-native pronunciation apps explain in Japanese but often provide weaker diagnosis and weaker corrective feedback.

The app should connect those two strengths:

**Speak → diagnose exact error → explain in natural Japanese → show the physical correction → targeted drill → retest → remember recurring weaknesses.**

## Locked App Store positioning

### Name

**英語発音コーチ｜日本人のための発音矯正**

Why this name:

- contains the high-intent Japanese search terms `英語`, `発音`, `発音矯正`, and `コーチ`;
- immediately identifies the Japanese-speaker specialization;
- describes the product plainly without depending on an invented brand;
- stays within Apple's 30-character app-name limit.

### Subtitle

**RとL・THなど苦手な音を診断、直し方まで日本語で**

The subtitle states the missing benefit found repeatedly in reviews: diagnosis alone is insufficient; users want to know exactly how to correct the sound.

### Canonical listing source

The complete App Store copy, keyword field, image captions, StoreKit metadata, privacy answers, review notes and submission checklist live in [`docs/app-store-listing-ja-JP.md`](app-store-listing-ja-JP.md). The machine-readable validation source is [`docs/app-store-listing-manifest.json`](app-store-listing-manifest.json).

Those files are canonical for all public listing fields. This product specification defines product behavior; it must not carry a second copy of promotional text or the description that can drift from the launch listing.

## Japan App Store supply audit — 2026-09-28

The audit covered the material direct pronunciation apps plus the large Japanese speaking apps that compete for the same budget. Ratings and visible Japanese prices were checked on the live Japan App Store pages. Recent visible reviews were read closely, with extra weight given to low-star and specific improvement requests.

| App | Japan rating signal | Visible pricing | What users value | Repeated weakness or recent complaint |
| --- | ---: | ---: | --- | --- |
| [ELSA Speak](https://apps.apple.com/jp/app/id1083804886) | 4.6, 63k ratings | Premium around ¥19,000/year; several legacy price points | detailed feedback and large curriculum | recognition rejects valid speech, slow responses, confusing flow, bugs, weak support, unclear entitlement changes and purchase restoration |
| [Speak](https://apps.apple.com/jp/app/id1286609883) | 4.5, 78k ratings | ¥3,800/month or ¥19,800/year; Plus ¥29,800/year | very high speaking volume, structured courses, practical AI conversation | unclear plan differences and price presentation, recognition variability, broad curriculum can feel repetitive or disconnected from free conversation |
| [SpeakBuddy](https://apps.apple.com/jp/app/id1129621266) | 4.6, 63k ratings | ¥3,980/month or ¥28,400/year | Japanese-first structure, short daily lessons, useful scenarios, low social pressure | pronunciation is one feature inside a broad course; users still need a clearer explanation of how to fix a detected sound |
| [BoldVoice](https://apps.apple.com/jp/app/id1567841142) | 4.7, 2,179 ratings | about ¥22,000/year | the strongest direct competitor for detailed phoneme feedback; good human-made lessons and automatic replay | recent Japanese reviews cite unnatural Japanese, English-only corrective explanations, sensitive audio input, onboarding/assessment bugs and high price |
| [英語発音トレーニング](https://apps.apple.com/jp/app/id1497634724) | 4.6, 9,941 ratings | free with ads | simple 5-minute practice, immediate result, easy model/self playback | scoring can accept deliberately wrong sounds or reject correct speech; users request IPA, better recording volume and British pronunciation support |
| [発音博士](https://apps.apple.com/jp/app/id885771578) | 3.6, 759 ratings | word packs from ¥160; larger packs ¥1,000–¥1,200 | shows what phoneme the learner produced, retains four recordings, clear model/self comparison | scoring can be arbitrary or excessively strict, frequent “could not hear” failures, no concrete physical correction, last visible update was in 2022 |
| [Speakometer](https://apps.apple.com/jp/app/id1529508890) | 4.7, 323 ratings | ¥500/month or ¥3,000/year | automatic replay, US/UK options, weakness identification and IPA | recent reviews request mouth-position guidance and report mismatched word/audio, weak Japanese translation and recognition errors |
| [Accent Training](https://apps.apple.com/jp/app/id1103315286) | 4.6, 236 ratings | ¥300/month or ¥3,000/year; ¥800 ad removal | rhythm, stress and intonation curriculum; short lessons and offline support | English-only positioning; recent reviews report a progression-blocking lesson bug and ads remaining after purchase |
| [Pronuncian](https://apps.apple.com/jp/app/id1436241871) | 4.6, 134 ratings | small niche product | phoneme reference content | recent Japanese review says buttons and audio do not work; other reviews cite incorrect TH mapping and crashes |
| [Say It](https://apps.apple.com/jp/app/id919978521) | 4.1, 68 ratings | in-app purchase/subscription history | waveform-style visual comparison | purchase-model changes damaged trust; repeated restore, crash and unusable-content complaints |

Additional direct listings such as LearnEnglish Sounds Right and Speakerly have only single-digit Japan rating evidence. They do not materially validate demand or change the product direction.

### Conclusions from the review evidence

1. **Exact diagnosis attracts users, but correction guidance creates willingness to pay.** Users repeatedly say they can see that a sound is wrong but still do not know how to fix it.
2. **Japanese pedagogy is the primary opening.** BoldVoice is closest to the desired product, yet a recent Japanese reviewer explicitly said the English explanation prevented progress and that they would pay if this were fixed.
3. **Trust must be designed into scoring.** The same complaint appears across ELSA, 発音博士, 英語発音トレーニング and Speakometer: valid speech is rejected, wrong speech passes, or the microphone stops early.
4. **Show uncertainty and the microphone state.** Display input level, wait for the full utterance, explain low confidence, and allow an immediate retry without penalty.
5. **Remember error patterns, not only best scores.** Reviews ask for recent-attempt averages, recurring phoneme-error rankings and focused practice based on those patterns.
6. **Reference and self playback must be one tap and volume matched.** This is one of the most consistently praised simple features and one of the clearest usability complaints when implemented poorly.
7. **IPA, slow playback and accepted accents matter.** Show standard IPA, provide slow reference audio and distinguish intelligibility from optional American-accent refinement.
8. **Keep the flow linear and short.** A learner should always know the next action: listen, record, understand, drill, retest.
9. **Pricing must be visible before onboarding.** Confusing subscriptions and changing entitlements create some of the strongest negative reviews in the category.
10. **The product should remain narrowly specialized.** Competing directly with the conversation breadth of Speak or SpeakBuddy would increase cost while weakening the clearest differentiation.

## Evidence behind the decision

### Demand and willingness to pay

The Japanese App Store already contains very large paid-speaking/pronunciation audiences:

- ELSA Speak — tens of thousands of Japanese ratings.
- Speak — tens of thousands of Japanese ratings and high annual subscription pricing.
- SpeakBuddy — tens of thousands of Japanese ratings and a Japan-specific English-learning audience.
- BoldVoice — thousands of Japanese ratings and premium annual pricing.
- Japanese-native pronunciation trainers also have meaningful rating volume, proving demand specifically for pronunciation rather than only general conversation practice.

### Localization / pedagogy gap

Recent Japanese BoldVoice feedback praises the pronunciation analysis but says critical corrective explanations are difficult because they are delivered in English or unnatural Japanese. One user explicitly indicated willingness to pay if that problem were solved.

Japanese-native apps show the reverse problem: users can receive a pronunciation score or error indication but still ask **how to physically correct the sound**.

Therefore the moat is not translation. It is **Japanese-native pronunciation pedagogy**.

## Product promise

**「何が違うか」だけでなく、「どう直すか」まで日本語で。**

The app should tell the learner:

1. what sound was expected;
2. what they most likely produced instead;
3. why Japanese speakers commonly make that substitution;
4. what to do with the tongue, lips, jaw, airflow, voicing, stress, or rhythm;
5. a minimal-pair drill;
6. a word drill;
7. a sentence drill;
8. whether the retest improved.

## Initial Japanese-speaker error model

Priority targets:

- /r/ ↔ /l/
- /θ/ and /ð/ ↔ /s/, /z/, /d/
- /b/ ↔ /v/
- /f/ ↔ /h/
- Japanese vowel insertion after English consonants
- word-final consonant deletion or weakening
- consonant clusters
- schwa /ə/
- lexical stress
- sentence stress
- mora-timed Japanese rhythm vs stress-timed English rhythm
- linking and reductions

The first release should stay narrow. Accuracy on a small set of high-value Japanese-speaker errors is preferable to superficial coverage of every English phoneme.

## Core process flow

### 1. Initial assessment

The learner records a short diagnostic set specifically designed to expose common Japanese-speaker English errors.

Output:

- strongest sounds;
- recurring substitutions;
- missing/deleted sounds;
- stress/rhythm issues;
- recommended drill order.

### 2. Daily training

Each session should be short:

1. hear target;
2. see concise Japanese explanation;
3. record;
4. analyze;
5. show detected error;
6. show corrective cue;
7. perform minimal-pair drill;
8. perform word/sentence drill;
9. retest;
10. update weakness history.

### 3. Error memory

Store recurring errors as first-class data, not merely aggregate scores.

Example:

- `/r/ → /l/`: 14 detections
- `/θ/ → /s/`: 9 detections
- final `/t/` omitted: 7 detections

This becomes the learner's personalized training queue.

### 4. Scoring philosophy

Do not reduce pronunciation to a single arbitrary "native accent" score.

Separate:

- **Intelligibility** — would the intended word likely be understood?
- **Target-sound accuracy** — was the intended phoneme produced?
- **Accent refinement** — optional finer improvement.

The product should optimize useful communication before accent imitation.

---

# Locked Technical Architecture

## Deployment target

- **Minimum:** iOS 18
- **App language:** Swift 6
- **UI:** SwiftUI using the native iOS 18 adaptive shell
- **Shared pronunciation core:** C++17 static library exposed through a narrow C/Swift wrapper
- **Model runtime:** ONNX Runtime Mobile with the Core ML execution provider
- **Audio:** AVFoundation / AVAudioEngine
- **Signal processing:** Accelerate / vDSP
- **Persistence:** SwiftData
- **Purchases:** StoreKit 2
- **Architecture:** engine-first; the pronunciation engine remains independent from presentation, persistence and StoreKit
- **Runtime:** entirely on-device for launch; no backend, API key, login or paid inference service

Do not require iOS 26 for the core product.

## UI layer

### SwiftUI

Use native SwiftUI components for:

- assessment flow;
- lessons;
- recording states;
- feedback;
- history;
- progress;
- settings;
- StoreKit purchase UI.

Use the value-based iOS 18 `TabView`/`Tab` APIs with `.sidebarAdaptable` for durable top-level destinations. Each data-oriented destination uses an adaptive `NavigationSplitView`: one column in compact width and two columns whenever iPad width permits. Use `NavigationStack` only for genuine drill-down inside a column.

Avoid custom controls where native iOS behavior is sufficient.

For App Store screenshots, the iPad set must use the **regular-width two-column `NavigationSplitView` presentation** with both columns visible. Do not capture the compact single-column fallback for the iPad product page; that duplicates the iPhone presentation and fails to demonstrate the adaptive iPad experience.

## Audio capture

### AVFoundation / AVAudioEngine

Use:

- `AVAudioSession`
- `AVAudioEngine`
- `AVAudioInputNode`
- PCM audio buffers

Responsibilities:

- microphone permission;
- high-quality mono capture;
- input-level monitoring;
- silence clipping;
- recording state;
- playback of learner recording;
- reference-audio playback.

Audio processing must be deterministic and independently testable.

## Speech-to-text

### iOS 18+

Use `SFSpeechRecognizer` only as a secondary intelligibility check where transcription is useful. Set `requiresOnDeviceRecognition = true` and check `supportsOnDeviceRecognition` at runtime. If the device cannot perform the check locally, skip this secondary signal and continue with the bundled phoneme engine.

It is **not** the pronunciation-scoring engine.

### iOS 26+

Progressively enhance with:

- `SpeechAnalyzer`
- `SpeechTranscriber`
- `AssetInventory`

Apple's newer speech stack performs on-device transcription and manages downloadable speech models in system storage.

Again: transcription validates the likely word/phrase. It does not by itself provide the phoneme-level corrective diagnosis required by this product.

## Pronunciation engine

### Core design

Use a dedicated, replaceable pronunciation-analysis module.

Input:

- normalized PCM audio;
- expected word/phrase;
- expected phoneme sequence;
- learner context.

Output:

- aligned phonemes;
- per-phoneme confidence;
- likely substitutions;
- deletions;
- insertions;
- duration/timing;
- stress/rhythm features;
- intelligibility signal;
- corrective lesson identifier.

### Runtime inference

Run a bundled quantized ONNX pronunciation model through **ONNX Runtime Mobile**, using the Core ML execution provider where supported and the CPU execution provider as a tested fallback.

Advantages:

- no per-request API charge;
- no API key or network dependency;
- no account requirement;
- private recordings;
- offline drills;
- one model/runtime path that can later be reused on Android;
- predictable versioning independent of Apple's transcription behavior;
- fits WorksBien's local-first architecture.

The locked checkpoint is `onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX`, which is Apache-2.0 licensed and emits phonetic labels from 16 kHz audio. The portable benchmark completed on 2026-09-28:

- **Selected:** `model_q4f16.onnx` — 196.9 MB;
- **Rejected:** `model_int8.onnx` — 317.7 MB.

Q4-FP16 was 38.0% smaller, used 20.3% less portable peak process RSS, and was 15.7% faster at median inference and 23.9% faster at p95 than INT8. Against the uncompressed FP32 reference, Q4-FP16 achieved 98.43% weighted frame agreement and 0.090 mean normalized decode edit distance over a speaker-balanced 250-utterance SpeechOcean762 subset; INT8 achieved 96.21% and 0.206. Q4-FP16 is therefore the only model to carry into the iOS engine. See `japanese-pronunciation-model-benchmark-2026-09-28.md` for the complete method and results.

Ship **one** model only. Do not bundle the full multi-variant model repository. Do not silently download a replacement model after release.

The shared C++17 pronunciation core owns normalization, CTC forced alignment, phoneme scoring, prosody feature extraction and deterministic decision rules. Swift owns microphone/session integration and maps the engine's typed result into app state. The engine repository contains no SwiftUI, SwiftData or StoreKit code.

### Model strategy

Do **not** make the first release dependent on a generic speech-to-text score.

Locked engine pipeline:

1. capture and convert microphone input to normalized 16 kHz mono PCM;
2. reject silence, clipping, incomplete speech and unusable signal-to-noise conditions before scoring;
3. produce phoneme logits with the bundled Wav2Vec2 phoneme CTC model;
4. force-align those logits against the exercise's prevalidated expected phoneme sequence;
5. calculate per-phoneme expected-versus-competing posterior scores using a GOP-style method;
6. compare only against the Japanese-relevant confusion set for that exercise;
7. calculate duration, energy, pitch, stress and rhythm features with Accelerate/vDSP;
8. optionally add the on-device Apple Speech intelligibility signal;
9. fuse the signals conservatively and return a typed confidence level;
10. map only high-confidence repeated errors to deterministic Japanese corrective guidance.

Never convert Apple transcription confidence into a pronunciation score. Never compare the learner's raw waveform directly with one reference speaker as the primary score. Never diagnose a specific physical error when the evidence is below the locked confidence threshold; return a neutral retry result instead.

The lesson text should not be generated unpredictably at runtime.

### Why not Apple Sound Analysis alone?

Sound Analysis supports custom Core ML sound classifiers and live audio streams, but Apple's built-in classifier is for broad sounds such as speech, music, laughter, etc.

It is useful infrastructure, not a ready-made English phoneme assessor.

If useful, Sound Analysis may host a custom classifier for narrow contrast tasks, but the core pronunciation engine should remain its own abstraction.

## Model development stack

Offline/model-development tooling:

- Python
- PyTorch
- torchaudio
- NumPy
- librosa only if required during research or validation
- ONNX export and quantization tooling
- ONNX Runtime desktop for parity testing
- coremltools only for optional comparison/conversion experiments
- pytest for model-pipeline tests

None of this ships as Python in the iOS app.

Candidate acoustic models must be benchmarked for:

- Japanese-accented English;
- phoneme alignment accuracy;
- device latency;
- ONNX Runtime iOS compatibility and Core ML execution-provider coverage;
- model size;
- license suitability for commercial distribution.

The Wav2Vec2 checkpoint, Q4-FP16 artifact and ONNX Runtime are locked. The portable model-selection benchmark is complete, but this is not automatic release approval. The automated engine gates below, together with the developer's own hands-on testing on a physical iPhone, determine whether the selected model may ship.

### Commercial-use and dependency rules

- ONNX Runtime Mobile is permitted under the MIT licence.
- The selected Wav2Vec2 phoneme checkpoint is permitted under Apache-2.0.
- Include all required open-source notices in the application bundle and repository.
- Do not train the commercial product with UME-ERJ unless separate commercial permission is obtained; its published licence is research-only.
- Do not embed or dynamically link eSpeak-ng. The fixed launch curriculum does not need runtime grapheme-to-phoneme conversion.
- Do not add a server SDK, analytics SDK, advertising SDK or remote model API to the pronunciation path.

## Expected pronunciation representation

Use a fixed ARPAbet-like internal inventory mapped to IPA for display. Every launch prompt has a manually reviewed phoneme sequence and accepted variants in versioned data.

Maintain:

- word → canonical phoneme sequence;
- accepted pronunciation variants;
- Japanese-relevant confusion pairs;
- articulatory instructions;
- minimal pairs;
- example words;
- sentence drills.

This content should be versioned data, not hard-coded throughout Swift files.
There is no runtime grapheme-to-phoneme dependency in V1.

## Content format

Preload lessons as JSON.

Suggested objects:

- `Phoneme`
- `PronunciationVariant`
- `JapaneseErrorPattern`
- `CorrectionGuide`
- `MinimalPair`
- `PracticeWord`
- `PracticeSentence`
- `DiagnosticPrompt`
- `Lesson`
- `AssessmentRule`

Example relationship:

`JapaneseErrorPattern(/r/ -> /l/)`
→ Japanese explanation
→ tongue-position instruction
→ minimal pairs
→ targeted words
→ sentences
→ success threshold.

This enables content expansion without rewriting the engine.

## Persistence

### SwiftData

Store locally:

- learner profile;
- assessment results;
- lesson progress;
- attempts;
- recurring phoneme errors;
- scores;
- streak/history;
- purchase state cache.

Do not retain raw recordings indefinitely by default.

Recommended default:

- process recording;
- retain only derived pronunciation result;
- discard raw audio after feedback unless the user explicitly chooses to keep examples.

This minimizes storage and privacy exposure.

## Sync / backup

V1 can be fully device-local.

If sync is added:

- native iCloud / CloudKit only;
- no WorksBien account system;
- no custom backend unless future economics justify it.

## Reference audio

Use Apple's on-device `AVSpeechSynthesizer` as the launch reference-audio engine. Do not make launch dependent on commissioned speakers, paid audio, a remote speech API or a bundled third-party recording corpus.

Each exercise stores a manually reviewed display string, expected phoneme sequence and IPA pronunciation. Construct an attributed `AVSpeechUtterance` using `AVSpeechSynthesisIPANotationAttribute` where spelling alone may produce an ambiguous or unsuitable pronunciation.

Voice selection:

- request an appropriate installed `en-US` system voice;
- prefer premium, then enhanced, then the basic on-device voice;
- never depend on one voice identifier being present on every device;
- fall back deterministically to the available `en-US` system voice;
- provide normal and slow instructional playback using separately configured utterances.

Use `AVSpeechSynthesizer.write(_:toBufferCallback:)` only when an audio buffer is needed for a waveform, temporary replay or testing. Do not permanently generate and bundle thousands of audio files.

Reference playback is instructional and must be labelled **model pronunciation**, not a human native-speaker recording. The pronunciation-scoring engine compares learner evidence with the exercise's validated phoneme sequence; it never scores by waveform similarity to the synthetic voice. A change in the installed Apple voice must therefore not change the expected answer or diagnosis.

The model-pronunciation layer has no API key, network dependency, per-use cost or meaningful additional app footprint.

## Japanese instructional content

All core Japanese explanations should be authored/reviewed as Japanese instructional content, not translated mechanically from English.

The content layer is part of the product moat.

Each correction should cover:

- what the user likely did;
- why Japanese speakers tend to do it;
- physical articulatory correction;
- a short analogy where useful;
- contrast sound;
- minimal pair;
- progressive drill.

## Monetization

### Recommended launch model

Free:

- complete initial pronunciation assessment and weakness map, excluded from the daily quota;
- the complete sound module selected from the learner's highest-confidence detected weakness;
- 10 valid scored recordings per local calendar day;
- unlimited playback of model pronunciation and access to the unlocked IPA and Japanese instructional content;
- a visible remaining-use counter and exact next-reset time;
- no accumulation of unused daily recordings.

Only a recording that passes the audio-quality gate and returns a pronunciation result consumes one use. Silence, clipping, interrupted recordings, permission failures and internal scoring failures do not consume the allowance.

Paid:

- full Japanese-speaker pronunciation curriculum;
- complete weakness tracking;
- personalized drill queue;
- all advanced stress/rhythm/linking lessons.

Locked launch offer:

**Pro subscription: ¥600/month or ¥4,800/year. No advertising.**

The annual plan is the primary offer. It costs the equivalent of ¥400 per month and gives the product predictable revenue for model validation, Japanese lesson maintenance, new drills and operating-system compatibility work.

Reason:

- the pronunciation model requires continuing validation and tuning across devices, voices, accents and iOS releases;
- Japanese corrective lessons, drill sets and accepted-pronunciation variants will continue to expand;
- personalized error history and daily training provide continuing value rather than a single-use utility;
- transparent pricing directly answers a major complaint across Speak, ELSA and Say It;
- ¥4,800/year remains materially below the ¥19,000–¥29,800 annual speaking products;
- the free diagnostic proves recognition quality before the learner subscribes;
- subscription revenue supports maintenance without adding distracting advertising or selling user data.

Do not require a payment method during onboarding. Let the learner complete the assessment and use the free product first. Do not add an auto-renewing free trial at launch: the renewable daily allowance is the trial. The free tier must be genuinely useful, while its scoring volume remains insufficient for intensive repetition across minimal pairs, words and sentences.

### Why advertising is rejected

- microphone practice needs concentration and a predictable audio session;
- interstitial or rewarded ads would interrupt the listen-record-review-retry loop;
- advertising revenue requires substantially more active users than a focused paid product;
- an advertising SDK weakens the local-first privacy position;
- ads would position the product beside the free, lower-trust practice apps rather than as a specialized correction tool.

## Payments

- StoreKit 2
- one subscription group with monthly and annual Pro products
- transaction verification using native StoreKit APIs
- restore purchases
- no account required

## Privacy

Required permissions:

- microphone
- speech recognition only if Apple transcription is enabled

Privacy position:

- recordings analyzed on device wherever practical;
- raw audio discarded by default;
- no advertising SDK;
- no third-party behavioral analytics required;
- no login required.

## Testing

### Engine tests

- phoneme alignment fixtures;
- substitution detection;
- insertion/deletion detection;
- silence/noise handling;
- clipped recording;
- wrong phrase;
- very quiet recording;
- retries;
- scoring stability;
- deterministic rule selection;
- migration of result schema.

### Audio corpus tests

Create a controlled validation corpus including:

- native English reference speakers;
- Japanese speakers across proficiency levels;
- intentional R/L, TH/S, B/V and vowel-insertion errors;
- male/female voices;
- different iPhone microphones;
- quiet and moderate-noise environments.

The engine should be judged primarily on whether it identifies **actionable Japanese-speaker errors correctly**, not whether an arbitrary overall score matches a commercial competitor.

### Usability tests

Universal:

- obvious recording state;
- immediate replay;
- retry without penalty;
- undo/recovery;
- no dead ends;
- explain low-confidence analysis;
- never pretend certainty.

App-specific:

- correction understandable without English linguistic knowledge;
- drill immediately addresses detected error;
- feedback does not shame accent;
- improvement can be perceived within a session;
- recurring errors remain visible.

## Repository structure

Recommended split:

```text
pronunciation-engine/
  CMakeLists.txt
  include/
    PronunciationEngine.h
  src/
    AudioQuality/
    CTCAlignment/
    PhonemeScoring/
    Prosody/
    DecisionRules/
    ContentValidation/
  model/
    model-manifest.json
    checksums.txt
  Resources/
    phonemes.json
    japanese-error-patterns.json
    lessons/
  Tests/
    Unit/
    Property/
    Corpus/
    Regression/
    Performance/
  Harness/

pronunciation-ios/
  App/
  Features/
    Assessment/
    Practice/
    Results/
    Progress/
    Settings/
  Audio/
  EngineBridge/
  Persistence/
  Store/
  Resources/
```

The engine repository contains no UI, SwiftData or StoreKit behavior. The selected model binary is versioned by checksum and distributed with the iOS target; do not commit multiple model variants to the production application.

## V1 exclusions

Do not build at launch:

- open-ended AI conversation;
- grammar tutor;
- vocabulary course;
- human tutoring marketplace;
- social feed;
- leaderboards;
- web app;
- Android version;
- WorksBien account system;
- server-side audio archive;
- generic "speak anything" analysis.

V1 should be exceptionally good at one thing:

> helping a Japanese speaker understand exactly why a specific English sound is wrong and how to physically correct it.

## Technical risk and mandatory pre-UI gate

The material engineering risks are pronunciation-model trustworthiness and the mobile latency/memory envelope. Q4-FP16 has passed the portable selection benchmark and INT8 has been rejected. Performance and diagnostic accuracy on a real device are verified by the developer's own hands-on testing rather than by a specification gate.

Launch validation uses reproducible automated corpora, deterministic engine tests and the developer's own hands-on testing on a physical iPhone. A paid speaker study is not a launch dependency. The free tier is the post-launch product-proof layer, but it does not replace pre-release correctness testing.

The engine may proceed to UI only when all of these are met:

- the selected acoustic model passes the locked FP32-parity regression over the public UME-ERJ samples and speaker-balanced SpeechOcean762 subset;
- posterior-level fixtures for every supported substitution, deletion and insertion produce the intended deterministic classification;
- unusable/silent/clipped audio is rejected rather than scored at least 95% of the time;
- low-confidence evidence produces `retry`, never a definitive physical correction;
- repeated-error history is updated only after the same high-confidence pattern is observed at least twice;
- the selected compressed model passes regression parity and licence/notice checks.

Without a labelled Japanese-speaker holdout study, marketing must not describe the engine as clinically validated or guarantee that every accent error will be detected. Post-launch complaints must be reproducible locally before rule or threshold changes are made, and thresholds must never be relaxed merely to produce more feedback.

If Q4-FP16 misses the remaining gate, do not compensate with UI or relax the threshold after seeing results. Narrow the launch contrast set, distil/fine-tune a smaller commercially permitted model, or block the build.

## Recommended implementation order

1. ~~Define phoneme inventory and Japanese confusion map.~~ **Complete (draft):** `pronunciation-engine/Resources/phonemes.json` and `japanese-error-patterns.json`; not yet linguistically reviewed.
2. ~~Build JSON content schema.~~ **Complete (draft):** `pronunciation-engine/Resources/*.json` + `src/Content`; covers the launch contrast set's priority targets. Not yet linguistically reviewed.
3. ~~Build the plain C++ engine harness.~~ **Complete:** `pronunciation-engine/Harness/main.cpp` (`gate` and `run` subcommands); see `pronunciation-engine/README.md`.
4. ~~Benchmark Q4-FP16 and INT8.~~ **Complete:** Q4-FP16 selected; INT8 rejected by the 2026-09-28 portable benchmark.
5. ~~Implement CTC forced alignment and GOP-style phoneme scoring.~~ **Complete:** `pronunciation-engine/src/CTCAlignment`, `src/PhonemeScoring`.
6. ~~Implement audio-quality and Accelerate/vDSP prosody analysis.~~ **Complete (portable reference; vDSP substitution pending iOS target):** `pronunciation-engine/src/AudioQuality`, `src/Prosody`.
7. ~~Implement conservative signal fusion and deterministic error classification.~~ **Complete:** `pronunciation-engine/src/DecisionRules`.
8. ~~Assemble the automated posterior-fixture and audio-quality regression suite.~~ **Complete against synthetic/mock evidence:** `pronunciation-engine/Tests/Unit` (45 tests, including a 200-sample synthetic audio-rejection sweep) + `pronunciation-harness gate` (45/45 scenarios). The public-corpus (UME-ERJ / SpeechOcean762) regression still needs to be repeated through the bundled Apple runtime.
9. ~~Wire the selected Q4-FP16 model into iOS.~~ **Complete:** ONNX Runtime 1.30.0, checksum-verified model bundling, 392-label model-output projection, C++/Swift bridge and Release engine factory are connected. Latency, memory, offline behaviour and the native-English pass on a physical iPhone are verified by the developer's own hands-on testing, not by a specification gate.
10. Write and validate Japanese corrective content.
11. Implement assessment and daily-practice state machines.
12. Add SwiftData persistence and StoreKit 2 entitlements.
13. Apply the native iOS 18 adaptive shell.
14. TestFlight with Japanese-language reviewers.

## Apple platform references

- Speech framework: https://developer.apple.com/documentation/Speech
- SpeechAnalyzer / SpeechTranscriber: https://developer.apple.com/videos/play/wwdc2025/277/
- AVFoundation: https://developer.apple.com/av-foundation/
- Accelerate: https://developer.apple.com/accelerate/
- Core ML: https://developer.apple.com/documentation/coreml
- Sound Analysis: https://developer.apple.com/documentation/SoundAnalysis
- StoreKit: https://developer.apple.com/storekit/
- ONNX Runtime Mobile: https://onnxruntime.ai/docs/tutorials/mobile/
- ONNX Runtime Core ML execution provider: https://onnxruntime.ai/docs/execution-providers/CoreML-ExecutionProvider.html
- Locked phoneme-model candidate: https://huggingface.co/onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX
- Model-selection benchmark: `japanese-pronunciation-model-benchmark-2026-09-28.md`
- UME-ERJ licence restriction: https://research.nii.ac.jp/src/en/UME-ERJ.html

## Final recommendation

**BUILD.**

The technical stack is viable without a mandatory server or paid runtime AI API.

The locked architecture is:

**SwiftUI + AVFoundation + C++17 pronunciation core + quantized Wav2Vec2 phoneme CTC model + ONNX Runtime Mobile/Core ML execution provider + CTC forced alignment/GOP-style scoring + Accelerate/vDSP prosody + deterministic Japanese pedagogy + SwiftData + StoreKit 2**

Apple's Speech framework is an optional on-device intelligibility signal, never the pronunciation assessor. No API key, server, login or paid runtime service is required.

The engine must be proven before UI work begins.
