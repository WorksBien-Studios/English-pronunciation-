#!/bin/bash
set -euo pipefail

REPO_ROOT="${SRCROOT}/.."
MODEL="${REPO_ROOT}/pronunciation-engine/model/model_q4f16.onnx"
RESOURCES="${REPO_ROOT}/pronunciation-engine/Resources"
DESTINATION="${TARGET_BUILD_DIR}/${UNLOCALIZED_RESOURCES_FOLDER_PATH}/PronunciationEngineResources"

python3 "${REPO_ROOT}/scripts/fetch_pronunciation_model.py" --destination "${MODEL}" --verify-only

rm -rf "${DESTINATION}"
mkdir -p "${DESTINATION}"
cp "${RESOURCES}"/*.json "${DESTINATION}/"
cp "${MODEL}" "${DESTINATION}/model_q4f16.onnx"

echo "Bundled verified pronunciation engine resources at ${DESTINATION}"
