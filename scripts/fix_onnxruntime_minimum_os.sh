#!/bin/bash
set -euo pipefail

FRAMEWORKS_PATH="${TARGET_BUILD_DIR}/${FRAMEWORKS_FOLDER_PATH}"
PLIST="${FRAMEWORKS_PATH}/onnxruntime.framework/Info.plist"

# ONNX Runtime's current iOS C/C++ archive may omit MinimumOSVersion when
# consumed through SwiftPM. Apple validates embedded frameworks separately.
# Patch only when the framework is present; static-link builds need no action.
if [[ -f "${PLIST}" ]]; then
  /usr/libexec/PlistBuddy -c "Delete :MinimumOSVersion" "${PLIST}" 2>/dev/null || true
  /usr/libexec/PlistBuddy -c "Add :MinimumOSVersion string ${IPHONEOS_DEPLOYMENT_TARGET}" "${PLIST}"
  echo "Set onnxruntime.framework MinimumOSVersion=${IPHONEOS_DEPLOYMENT_TARGET}"
fi
