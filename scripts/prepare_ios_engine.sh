#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CACHE_DIR="${PRONUNCIATION_MODEL_CACHE:-$ROOT/.model-cache}"
CACHE_MODEL="$CACHE_DIR/model_q4f16.onnx"
BUNDLE_DIR="$ROOT/ios/Generated/PronunciationEngine"
EXPECTED_SHA="9978e7cba1bf721699b2cb673be461ccf09f6ef5dfc257f2f6ed2c4113d4e54c"
EXPECTED_SIZE="196911845"
MODEL_URL="https://huggingface.co/onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX/resolve/0c57084/onnx/model_q4f16.onnx"

mkdir -p "$CACHE_DIR"

valid_model() {
  [[ -f "$1" ]] || return 1
  [[ "$(stat -f%z "$1" 2>/dev/null || stat -c%s "$1")" == "$EXPECTED_SIZE" ]] || return 1
  [[ "$(shasum -a 256 "$1" | awk '{print $1}')" == "$EXPECTED_SHA" ]]
}

if ! valid_model "$CACHE_MODEL"; then
  rm -f "$CACHE_MODEL" "$CACHE_MODEL.part"
  curl --fail --location --retry 3 --retry-all-errors     --output "$CACHE_MODEL.part" "$MODEL_URL"
  mv "$CACHE_MODEL.part" "$CACHE_MODEL"
  if ! valid_model "$CACHE_MODEL"; then
    echo "Pinned pronunciation model failed size/SHA-256 verification." >&2
    rm -f "$CACHE_MODEL"
    exit 1
  fi
fi

rm -rf "$BUNDLE_DIR"
mkdir -p "$BUNDLE_DIR"
cp -R "$ROOT/pronunciation-engine/Resources/." "$BUNDLE_DIR/"
cp "$CACHE_MODEL" "$BUNDLE_DIR/model_q4f16.onnx"

if ! valid_model "$BUNDLE_DIR/model_q4f16.onnx"; then
  echo "Bundled pronunciation model failed verification." >&2
  exit 1
fi

echo "Prepared frozen pronunciation bundle at $BUNDLE_DIR"
