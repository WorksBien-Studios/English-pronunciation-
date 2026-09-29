# Japanese Pronunciation Coach — Embedded Model Benchmark

**Benchmark date:** 2026-09-28  
**Decision:** Select `model_q4f16.onnx`; reject `model_int8.onnx`.

## Executive verdict

Q4-FP16 is the locked development and launch candidate. Against INT8 it is:

- 38.0% smaller on disk;
- 15.7% faster at median inference and 23.9% faster at p95 in the portable CPU benchmark;
- 20.3% lower in measured process peak RSS;
- substantially closer to the uncompressed FP32 reference on both public corpora.

INT8 offers no measured advantage relevant to this product. The choice is therefore not a size-versus-quality compromise: Q4-FP16 wins both deployment efficiency and compression fidelity.

This result closes the portable model-selection benchmark. It does **not** yet close the full product engine gate. Physical iPhone/Core ML measurements, the final scoring pipeline, automated diagnostic fixtures and the complete developer test remain mandatory.

## Candidates

All three files are from [`onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX`](https://huggingface.co/onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX), licensed Apache-2.0.

| Model | Role | File size | SHA-256 |
|---|---|---:|---|
| `model_q4f16.onnx` | Selected compressed candidate | 196,911,845 bytes | `9978e7cba1bf721699b2cb673be461ccf09f6ef5dfc257f2f6ed2c4113d4e54c` |
| `model_int8.onnx` | Rejected compressed candidate | 317,712,780 bytes | `74174710e34035bbb7f611601d016c32fc575de7a6f53b1078107dc10a84e7ae` |
| `model_fp32.onnx` | Uncompressed fidelity reference | 1,264,009,987 bytes | `93265694093f5f91497181ed9d7791f43bc818d2e23c46caf988fd9b8b1a1fba` |

Runtime: ONNX Runtime 1.30.0. Audio was decoded to normalized 16 kHz mono PCM. CTC output comparisons use per-frame argmax labels and collapsed phoneme sequences.

## Benchmark data

### UME-ERJ public samples

Eighteen public sample WAV files were evaluated: nine American-English and nine Japanese-English recordings, including the labelled `bap`/`bep` minimal pair. The full UME-ERJ corpus is restricted to research use; these public samples are diagnostic only and must not be redistributed with the product or used as commercial training data. See the [official licence page](https://research.nii.ac.jp/src/en/UME-ERJ.html).

### SpeechOcean762

A speaker-balanced subset of the official test split was used:

- 250 utterances;
- 125 speakers, with two utterances per speaker;
- lowest- and highest-scoring utterance selected for each speaker;
- human sentence-accuracy scores spanning 1–10;
- Mandarin-L1 English, so useful for non-native robustness but not a substitute for Japanese-specific validation.

SpeechOcean762 is distributed under CC BY 4.0 through [OpenSLR SLR101](https://www.openslr.org/101/). Corpus archive SHA-256: `876c34b828c2ebcc35ba2ff1ba5947d50e6b0162fc4a4af91db0942422b3bdd5`.

## Portable performance result

Each compressed model ran in three independent sessions over the 18 UME-ERJ public samples. Each recording was inferred three times per session. The host was Linux x86-64 using ONNX Runtime's CPU execution provider with four threads. Values below are medians across the three sessions.

| Metric | Q4-FP16 | INT8 | Winner |
|---|---:|---:|---|
| Model size | 196.9 MB | 317.7 MB | Q4-FP16 |
| Session load | 0.606 s | 0.587 s | INT8, by 0.019 s |
| Median inference | 0.232 s | 0.276 s | Q4-FP16 |
| p95 inference | 0.587 s | 0.771 s | Q4-FP16 |
| Peak process RSS | 563.9 MB | 707.4 MB | Q4-FP16 |
| Stable decoded output across trials | 18/18 | 18/18 | Tie |

These latency and memory values establish a reproducible portable baseline, not an iPhone claim. Core ML execution-provider coverage, thermal behaviour, and peak additional resident memory must be measured on the target device.

## Compression fidelity against FP32

### UME-ERJ public samples

| Metric | Q4-FP16 vs FP32 | INT8 vs FP32 |
|---|---:|---:|
| Mean frame agreement | 98.53% | 96.05% |
| Median frame agreement | 98.90% | 97.20% |
| Minimum frame agreement | 92.86% | 85.71% |
| Exact collapsed decode | 13/18 | 8/18 |
| Mean normalized decode edit distance | 0.093 | 0.187 |
| Mean logit MAE | 0.343 | 0.835 |

On the labelled American `bap` and `bep` examples, Q4-FP16 preserved the final consonant seen in FP32, while INT8 dropped it. Final-consonant weakening is a launch contrast, so this is directionally important, but two examples are not an accuracy claim.

### SpeechOcean762 speaker-balanced test subset

| Metric | Q4-FP16 vs FP32 | INT8 vs FP32 |
|---|---:|---:|
| Weighted frame agreement | 98.43% | 96.21% |
| Median per-sample agreement | 98.59% | 96.55% |
| Minimum per-sample agreement | 92.86% | 88.11% |
| Exact collapsed decode | 68/250 (27.2%) | 38/250 (15.2%) |
| Mean normalized decode edit distance | 0.090 | 0.206 |
| Mean logit MAE | 0.370 | 0.874 |

Fidelity by human sentence-accuracy band:

| Human score band | Samples | Q4 frame agreement | INT8 frame agreement | Q4 decode edit distance | INT8 decode edit distance |
|---|---:|---:|---:|---:|---:|
| Low (1–5) | 43 | 98.30% | 95.42% | 0.119 | 0.325 |
| Mid (6–7) | 66 | 98.56% | 95.98% | 0.074 | 0.218 |
| High (8–10) | 141 | 98.31% | 96.62% | 0.089 | 0.165 |

Q4-FP16's advantage is largest on low-scoring speech—the cases where the product most needs stable evidence before issuing a correction.

## Decision

Lock `model_q4f16.onnx` as the only model carried forward into the C++/iOS engine. Do not bundle or continue product integration work on `model_int8.onnx` unless a future runtime regression invalidates Q4-FP16.

The selection passes these model-level gates:

- commercially usable checkpoint licence;
- deterministic runtime completion on all evaluated files;
- smaller package and lower portable memory than INT8;
- better portable latency than INT8;
- materially better parity with the uncompressed model;
- no network service or API key required.

## Remaining mandatory gates

Before UI production or release approval:

1. Run the selected model through ONNX Runtime Mobile with the Core ML execution provider and CPU fallback on a physical iPhone SE 2020.
2. Confirm p95 end-to-end scoring latency of no more than 2.5 seconds for clips up to three seconds.
3. Confirm additional peak resident memory of no more than 650 MiB on that device.
4. Implement the actual CTC forced alignment, GOP-style contrast scoring, audio-quality rejection, and conservative confidence rules; this benchmark compares acoustic-model outputs, not final diagnoses.
5. Run the locked public-corpus regressions, posterior-level fixtures for every supported error class, audio-quality rejection suite and complete native-English developer pass over the launch curriculum. Low-confidence evidence must return `retry`, and persistent weaknesses require two high-confidence observations.
6. Verify airplane-mode operation, licences/notices, and a signed model checksum in the application bundle.

A labelled Japanese-speaker holdout study would strengthen later validation but is not a launch dependency. Until such a study exists, do not claim clinical validation or guaranteed detection of every accent error.

Until those checks pass, the correct status is: **model selected; pronunciation engine not yet release-approved**.
