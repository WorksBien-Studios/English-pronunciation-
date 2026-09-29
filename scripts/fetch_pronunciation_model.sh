#!/usr/bin/env bash
set -euo pipefail

readonly expected_sha="9978e7cba1bf721699b2cb673be461ccf09f6ef5dfc257f2f6ed2c4113d4e54c"
readonly expected_size="196911845"
readonly model_url="https://huggingface.co/onnx-community/wav2vec2-lv-60-espeak-cv-ft-ONNX/resolve/c69750f5043e5e1f8a71ab95dd3b98338c280c92/onnx/model_q4f16.onnx"
readonly repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
readonly destination="$repository_root/ios/EnglishPronunciationCoach/Resources/model_q4f16.onnx"

sha256_file() {
  if command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$1" | awk '{print $1}'
  else
    sha256sum "$1" | awk '{print $1}'
  fi
}

if [[ -f "$destination" ]] && \
   [[ "$(wc -c < "$destination" | tr -d ' ')" == "$expected_size" ]] && \
   [[ "$(sha256_file "$destination")" == "$expected_sha" ]]; then
  echo "Pronunciation model already verified."
  exit 0
fi

mkdir -p "$(dirname "$destination")"
temporary="$(mktemp "${TMPDIR:-/tmp}/model_q4f16.XXXXXX.onnx")"
trap 'test ! -e "$temporary" || mv "$temporary" "${temporary}.failed"' EXIT

curl --fail --location --retry 3 --retry-all-errors \
  --output "$temporary" "$model_url"

actual_size="$(wc -c < "$temporary" | tr -d ' ')"
actual_sha="$(sha256_file "$temporary")"
[[ "$actual_size" == "$expected_size" ]] || {
  echo "Model size mismatch: expected $expected_size, got $actual_size" >&2
  exit 1
}
[[ "$actual_sha" == "$expected_sha" ]] || {
  echo "Model checksum mismatch: expected $expected_sha, got $actual_sha" >&2
  exit 1
}

mv "$temporary" "$destination"
trap - EXIT
echo "Fetched and verified model_q4f16.onnx."
