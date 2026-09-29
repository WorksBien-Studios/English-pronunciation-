#!/usr/bin/env python3
"""Fetch and verify the locked Q4-FP16 pronunciation model.

The ~197 MB model is deliberately not stored in Git. Builds fetch it into the
ignored pronunciation-engine/model directory, verify size + SHA-256, and then
Xcode copies that exact artifact into the application bundle.
"""
from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import tempfile
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parent.parent
DEFAULT_DEST = ROOT / "pronunciation-engine" / "model" / "model_q4f16.onnx"
MODEL_URL = (
    "https://huggingface.co/onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX/"
    "resolve/main/onnx/model_q4f16.onnx?download=true"
)
EXPECTED_SIZE = 196_911_845
EXPECTED_SHA256 = "9978e7cba1bf721699b2cb673be461ccf09f6ef5dfc257f2f6ed2c4113d4e54c"


def digest(path: pathlib.Path) -> tuple[int, str]:
    sha = hashlib.sha256()
    size = 0
    with path.open("rb") as handle:
        while chunk := handle.read(1024 * 1024):
            size += len(chunk)
            sha.update(chunk)
    return size, sha.hexdigest()


def verify(path: pathlib.Path) -> None:
    if not path.is_file():
        raise SystemExit(f"model missing: {path}")
    size, checksum = digest(path)
    if size != EXPECTED_SIZE:
        raise SystemExit(f"model size mismatch: expected {EXPECTED_SIZE}, got {size}")
    if checksum != EXPECTED_SHA256:
        raise SystemExit(f"model SHA-256 mismatch: expected {EXPECTED_SHA256}, got {checksum}")


def fetch(destination: pathlib.Path) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists():
        try:
            verify(destination)
            print(f"Pronunciation model already verified: {destination}")
            return
        except SystemExit:
            destination.unlink()

    fd, tmp_name = tempfile.mkstemp(prefix="model_q4f16.", suffix=".download", dir=destination.parent)
    os.close(fd)
    temporary = pathlib.Path(tmp_name)
    try:
        request = urllib.request.Request(MODEL_URL, headers={"User-Agent": "WorksBien-EnglishPronunciationCoach/1.0"})
        with urllib.request.urlopen(request, timeout=120) as response, temporary.open("wb") as output:
            while chunk := response.read(1024 * 1024):
                output.write(chunk)
        verify(temporary)
        os.replace(temporary, destination)
        print(f"Fetched and verified pronunciation model: {destination}")
    finally:
        if temporary.exists():
            temporary.unlink()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--destination", type=pathlib.Path, default=DEFAULT_DEST)
    parser.add_argument("--verify-only", action="store_true")
    args = parser.parse_args()
    destination = args.destination.resolve()
    if args.verify_only:
        verify(destination)
        print(f"Pronunciation model verified: {destination}")
    else:
        fetch(destination)


if __name__ == "__main__":
    main()
