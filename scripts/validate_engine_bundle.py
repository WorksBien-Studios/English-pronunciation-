#!/usr/bin/env python3
"""Fail closed when iOS stage content and the frozen acoustic mapping drift."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ENGINE = ROOT / "pronunciation-engine"
IOS = ROOT / "ios" / "EnglishPronunciationCoach"

phoneme_doc = json.loads((ENGINE / "Resources" / "phonemes.json").read_text())
symbols = {item["symbol"] for item in phoneme_doc["phonemes"]}

words_doc = json.loads((ENGINE / "Resources" / "practice-words.json").read_text())
engine_words = {item["id"]: item for item in words_doc["practiceWords"]}
if len(engine_words) != len(words_doc["practiceWords"]):
    raise SystemExit("Duplicate practice-word id in engine content.")

stage_doc = json.loads((IOS / "Resources" / "stages.json").read_text())
stage_words = {
    word["text"].lower()
    for stage in stage_doc["stages"]
    for word in stage["words"]
}
missing = sorted(stage_words - set(engine_words))
if missing:
    raise SystemExit("Stage words missing from pronunciation engine: " + ", ".join(missing))

for word_id in sorted(stage_words):
    unknown = [p for p in engine_words[word_id]["phonemes"] if p not in symbols]
    if unknown:
        raise SystemExit(f"{word_id} uses unknown phonemes: {unknown}")

mapping = json.loads((ENGINE / "Resources" / "acoustic-token-map.json").read_text())
vocab_size = int(mapping["modelVocabSize"])
blank = int(mapping["blankTokenId"])
seen = {}
for entry in mapping["mappings"]:
    if not entry["phonemes"]:
        raise SystemExit("Acoustic mapping contains an empty phoneme target.")
    for symbol in entry["phonemes"]:
        if symbol not in symbols:
            raise SystemExit(f"Acoustic mapping references unknown phoneme: {symbol}")
    for token in entry["tokenIds"]:
        token = int(token)
        if token < 0 or token >= vocab_size:
            raise SystemExit(f"Acoustic token id out of range: {token}")
        if token == blank:
            raise SystemExit("CTC blank token must not map to an engine phoneme.")
        if token in seen:
            raise SystemExit(
                f"Acoustic token {token} is mapped more than once: "
                f"{seen[token]} and {entry['phonemes']}"
            )
        seen[token] = entry["phonemes"]

manifest = json.loads((ENGINE / "model" / "model-manifest.json").read_text())
expected = manifest["checksum"]["value"]
checksums = (ENGINE / "model" / "checksums.txt").read_text()
if not expected or expected not in checksums:
    raise SystemExit("Model manifest/checksums.txt are not locked to the same SHA-256.")
if manifest["selectedArtifact"]["sizeBytes"] != 196_911_845:
    raise SystemExit("Pinned model size does not match the verified artifact.")

print(
    f"Engine bundle valid: {len(stage_words)} stage words, "
    f"{len(seen)} mapped acoustic tokens, model SHA locked."
)
