# pronunciation-engine

The shared C++17 pronunciation-analysis core locked in
[`docs/product-specification.md`](../docs/product-specification.md). It owns
audio-quality gating, CTC forced alignment, GOP-style phoneme scoring,
prosody feature extraction, and conservative decision rules; it has no
SwiftUI, SwiftData, or StoreKit dependency (`pronunciation-ios` owns all of
that and links this as a static library through `include/PronunciationEngine.h`).

## Build

```sh
cmake -S . -B build
cmake --build build -j
ctest --test-dir build            # or: build/pronunciation_engine_tests
build/pronunciation-harness Resources gate
```

No network access or third-party model download is required to build, test,
or run the harness: `third_party/nlohmann/json.hpp` is vendored, and every
scenario runs against `MockAcousticModel`.

## What's implemented

Per the locked pipeline in `docs/product-specification.md#model-strategy`:

1. **Audio quality gate** (`src/AudioQuality`) — silence, clipping, too-short,
   and SNR rejection before any inference is attempted.
2. **Acoustic model interface** (`src/Acoustic`) — `AcousticModel` is the only
   thing the rest of the engine depends on. `MockAcousticModel` scripts
   deterministic frame log-probabilities for development, tests, and the
   harness. `OnnxAcousticModel` runs the locked
   `onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX` checkpoint through ONNX
   Runtime 1.30.0, normalizes/resamples input audio, and projects its 392
   eSpeak/IPA labels into the engine inventory without discarding unknown evidence.
3. **CTC forced alignment** (`src/CTCAlignment`) — Viterbi forced alignment
   of frame log-probabilities against an exercise's expected phoneme
   sequence, plus a free greedy decode + Levenshtein alignment used
   separately to detect phonemes outside the expected sequence (insertions).
4. **GOP-style phoneme scoring** (`src/PhonemeScoring`) — per-phoneme
   Goodness-of-Pronunciation score restricted to each slot's Japanese
   confusion set, classifying each aligned phoneme as correct, substituted,
   deleted, or low-confidence.
5. **Prosody** (`src/Prosody`) — portable duration/energy/pitch/stress
   features (autocorrelation pitch tracking, RMS-dB energy) written behind a
   pure-function interface so an Accelerate/vDSP-backed implementation can
   be substituted on the iOS target without touching any other module.
6. **Decision rules + error history** (`src/DecisionRules`) — the single
   place that turns evidence into Pass/Retry/SpecificError, gated by each
   Japanese error pattern's authored confidence threshold, plus the
   repeats-required-to-confirm history rule.
7. **Content layer** (`src/Content`, `src/ContentValidation`) — loads and
   cross-validates the versioned JSON content pack in `Resources/`
   (phoneme inventory, Japanese error patterns, minimal pairs, practice
   words/sentences, diagnostic prompts, lessons, assessment rules).
8. **Engine orchestration** (`src/Engine.h/.cpp`) and the **narrow C API**
   (`include/PronunciationEngine.h`, `src/CApi.cpp`) that Swift will link
   against.
9. **CLI harness** (`Harness/main.cpp`) — a `gate` subcommand that sweeps
   every practice word/pattern through correct-pronunciation and
   substitution scenarios (45/45 passing today), and a `run` subcommand for
   ad hoc one-off scripted exercises.

## Remaining release gates

- **Physical-device evidence is still required.** The backend, Apple runtime,
  verified model download, iOS bundle wiring and Swift adapter are complete,
  but latency, memory and Core ML/CPU partitioning still need measurement on
  the reference iPhone SE 2020 before release approval.
- **Content is an engineering draft**, not the "manually reviewed" content
  the spec's pre-UI gate requires. It is internally consistent (see
  `ContentValidator` and the `content_pack_passes_structural_validation`
  test) and covers the spike's priority targets (R/L, TH/DH, B/V, F/H, vowel
  insertion, final-consonant deletion, schwa), but a native-Japanese
  linguistic reviewer still needs to sign off on the explanations,
  articulatory instructions, and phoneme transcriptions before launch.
- **Accelerate/vDSP is not used.** `src/Prosody` is a portable reference
  implementation so the engine builds and tests on any host; the iOS target
  can substitute a vDSP-accelerated implementation behind the same function
  signature.

## Pre-UI gate status

Against `docs/product-specification.md#technical-risk-and-mandatory-pre-ui-gate`:

| Gate | Status |
| --- | --- |
| Every launch prompt has an expected phoneme sequence, IPA, accepted variants, confusion set | Draft content in place; not yet linguistically reviewed |
| Selected model passes FP32-parity regression | Done at the portable-benchmark level (`docs/model-benchmark-2026-09-28.md`); not yet re-run against the bundled on-device runtime |
| Posterior-level fixtures for every substitution/deletion/insertion produce the intended classification | ✅ — Unit tests + `pronunciation-harness gate` (45/45) |
| Correct native-English pronunciation never produces a high-confidence specific-error diagnosis | ✅ — exercised for every practice word by `pronunciation-harness gate` |
| Unusable/silent/clipped audio rejected ≥95% of the time | ✅ in unit tests against synthetic audio; not yet measured against a real recorded corpus |
| Low-confidence evidence → retry, never a definitive correction | ✅ — `DecisionRules` never emits `SpecificError` below a pattern's locked `confidenceThreshold` |
| Repeated-error history updates only after ≥2 high-confidence observations | ✅ — `ErrorHistory` + covering tests |
| p95 latency ≤2.5s / peak RSS ≤650MiB on iPhone SE 2020 for ≤3s recordings | Backend is wired; physical-device measurement remains |
| Works fully offline | ✅ by construction — no network calls anywhere in this engine |
| Compressed model passes regression parity and licence/notice checks | Licence notices in place (`THIRD_PARTY_NOTICES.md`); parity regression pending the real runtime |

The remaining rows require physical-device/corpus validation and linguistic
content review; they are no longer blocked on runtime or bundle integration.
