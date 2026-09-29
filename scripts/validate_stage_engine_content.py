#!/usr/bin/env python3
"""Fail if the app's stage words and the engine's content pack disagree.

Every word in ios/.../stages.json must exist in pronunciation-engine/Resources/practice-words.json
(otherwise the engine answers "unavailable"), must carry an error pattern, and its engine phoneme sequence must
contain the sound the stage says the learner is practising.
"""
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
STAGES = ROOT / "ios" / "EnglishPronunciationCoach" / "Resources" / "stages.json"
WORDS = ROOT / "pronunciation-engine" / "Resources" / "practice-words.json"
SOUND_PHONEME = {"r": "R", "l": "L", "th": "TH", "v": "V", "f": "F", "b": "B"}


def main():
    engine = {w["id"]: w for w in json.loads(WORDS.read_text(encoding="utf-8"))["practiceWords"]}
    stages = json.loads(STAGES.read_text(encoding="utf-8"))["stages"]
    problems = []
    seen = {}
    for stage in stages:
        for word in stage["words"]:
            text = word["text"]
            if text in seen:
                problems.append(f"{text}: appears in both {seen[text]} and {stage['id']}")
            seen[text] = stage["id"]
            entry = engine.get(text.lower())
            if entry is None:
                problems.append(f"{stage['id']}/{text}: missing from engine practice-words.json")
                continue
            if not entry.get("targetErrorPatternIds"):
                problems.append(f"{stage['id']}/{text}: engine entry has no targetErrorPatternIds")
            if word["target"] not in stage["sounds"]:
                problems.append(f"{stage['id']}/{text}: target {word['target']} not in stage sounds")
            wanted = SOUND_PHONEME[word["target"]]
            if wanted not in entry["phonemes"]:
                problems.append(f"{stage['id']}/{text}: engine phonemes {entry['phonemes']} lack {wanted}")
            if word.get("contrast") and word["contrast"] not in stage["sounds"]:
                problems.append(f"{stage['id']}/{text}: contrast {word['contrast']} not in stage sounds")
    if problems:
        sys.exit("Stage/engine content mismatch:\n  " + "\n  ".join(problems))
    print(f"{len(seen)} stage words across {len(stages)} stages all resolve in the engine content pack.")


if __name__ == "__main__":
    main()
