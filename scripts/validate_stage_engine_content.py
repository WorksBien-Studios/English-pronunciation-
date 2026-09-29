#!/usr/bin/env python3
"""Fail if iOS exposes a practice word the pronunciation engine cannot resolve."""
import json
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
STAGES = ROOT / "ios" / "EnglishPronunciationCoach" / "Resources" / "stages.json"
WORDS = ROOT / "pronunciation-engine" / "Resources" / "practice-words.json"


def load(path):
    return json.loads(path.read_text(encoding="utf-8"))


def main():
    stages = load(STAGES)["stages"]
    engine_words = load(WORDS)["practiceWords"]

    ids = [str(item["id"]).strip().lower() for item in engine_words]
    if len(ids) != len(set(ids)):
        duplicates = sorted({item for item in ids if ids.count(item) > 1})
        raise SystemExit("duplicate engine practice-word ids: " + ", ".join(duplicates))

    staged = []
    for stage in stages:
        for word in stage["words"]:
            staged.append(str(word["text"]).strip().lower())

    missing = sorted(set(staged) - set(ids))
    if missing:
        raise SystemExit(
            "iOS stage words missing from pronunciation-engine/Resources/practice-words.json: "
            + ", ".join(missing)
        )

    print(f"Stage/engine content coverage OK: {len(set(staged))} staged words are resolvable.")


if __name__ == "__main__":
    main()
