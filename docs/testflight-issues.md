# TestFlight issue ledger

Record every failed, cancelled, degraded, or unexpectedly expensive TestFlight run. A `fixed` entry must cite a later successful verifying run.

## TF-13D9A61A10 — app-manager-key-lacks-cloud-signing-access
<!-- testflight-issue-json: {"first_observed":"2026-09-30T00:22:04+00:00","fix":"Replaced the repository credentials with the dedicated Admin team API key 7ZJN3LKQSD and locked the expected key ID in the app map and repository variables","id":"TF-13D9A61A10","last_updated":"2026-09-30T01:29:52+00:00","minutes_wasted":3.0,"notes":"","prevention":"Use the dedicated least-scoped Admin delivery key for automatic cloud signing and verify the mapped key ID before any macOS runner work","productivity_minutes_lost":0.0,"repo":"lrodeveloperr/English-pronunciation-","root_cause":"The App Store Connect team API key had App Manager access, which could upload builds but could not create or use the cloud-managed distribution certificate and provisioning profile for this team","runs":["https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36649593171"],"signature":"app-manager-key-lacks-cloud-signing-access","stage":"export","status":"fixed","symptom":"Export failed because Apple denied cloud signing and no iOS Distribution certificate or App Store provisioning profile was available","verified_run":"https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723","workflow":"English Pronunciation Coach TestFlight","xcode_version":"26.6"} -->

- Status: `fixed`
- Repository / workflow: `lrodeveloperr/English-pronunciation-` / `English Pronunciation Coach TestFlight`
- Xcode / stage: `26.6` / `export`
- First observed: 2026-09-30T00:22:04+00:00
- Last updated: 2026-09-30T01:29:52+00:00
- Occurrences: 1
- Runner minutes wasted: 3.0
- Productivity minutes lost: 0.0
- Runs:
  - https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36649593171

**Symptom:** Export failed because Apple denied cloud signing and no iOS Distribution certificate or App Store provisioning profile was available

**Root cause:** The App Store Connect team API key had App Manager access, which could upload builds but could not create or use the cloud-managed distribution certificate and provisioning profile for this team

**Fix:** Replaced the repository credentials with the dedicated Admin team API key 7ZJN3LKQSD and locked the expected key ID in the app map and repository variables

**Verified by:** https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723

**Prevention:** Use the dedicated least-scoped Admin delivery key for automatic cloud signing and verify the mapped key ID before any macOS runner work

**Notes:** None

## TF-A6B1E3C9C1 — automatic-archive-requested-development-profile
<!-- testflight-issue-json: {"first_observed":"2026-09-30T00:16:17+00:00","fix":"Archive unsigned once, then perform one authenticated App Store Connect cloud export with Apple Distribution signing","id":"TF-A6B1E3C9C1","last_updated":"2026-09-30T01:29:52+00:00","minutes_wasted":2.0,"notes":"","prevention":"Keep archive and export split; fail closed on archive identity and exported distribution signatures","productivity_minutes_lost":0.0,"repo":"lrodeveloperr/English-pronunciation-","root_cause":"Automatic signing during archive selected a development identity on the hosted runner, which had no registered device or development profile","runs":["https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36649176916"],"signature":"automatic-archive-requested-development-profile","stage":"archive","status":"fixed","symptom":"Archive failed before IPA creation because Xcode requested an iOS App Development profile and a registered device","verified_run":"https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723","workflow":"English Pronunciation Coach TestFlight","xcode_version":"26.6"} -->

- Status: `fixed`
- Repository / workflow: `lrodeveloperr/English-pronunciation-` / `English Pronunciation Coach TestFlight`
- Xcode / stage: `26.6` / `archive`
- First observed: 2026-09-30T00:16:17+00:00
- Last updated: 2026-09-30T01:29:52+00:00
- Occurrences: 1
- Runner minutes wasted: 2.0
- Productivity minutes lost: 0.0
- Runs:
  - https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36649176916

**Symptom:** Archive failed before IPA creation because Xcode requested an iOS App Development profile and a registered device

**Root cause:** Automatic signing during archive selected a development identity on the hosted runner, which had no registered device or development profile

**Fix:** Archive unsigned once, then perform one authenticated App Store Connect cloud export with Apple Distribution signing

**Verified by:** https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723

**Prevention:** Keep archive and export split; fail closed on archive identity and exported distribution signatures

**Notes:** None

## TF-B4763B6B3C — nested-onnxruntime-framework-not-distribution-signed
<!-- testflight-issue-json: {"first_observed":"2026-09-30T00:52:35+00:00","fix":"Stage every embedded framework with a temporary ad-hoc signature before export, then require Apple Distribution authority and the mapped team on every exported framework","id":"TF-B4763B6B3C","last_updated":"2026-09-30T01:29:52+00:00","minutes_wasted":4.0,"notes":"","prevention":"Recursively verify nested framework signatures before upload and reject ad-hoc or wrong-team signatures","productivity_minutes_lost":0.0,"repo":"lrodeveloperr/English-pronunciation-","root_cause":"The unsigned archive left the embedded Swift-package framework without a signature; authenticated export did not repair a completely unsigned nested framework","runs":["https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36651942464"],"signature":"nested-onnxruntime-framework-not-distribution-signed","stage":"upload","status":"fixed","symptom":"App Store Connect rejected build 1 with error 90035 because onnxruntime.framework/onnxruntime was not distribution-signed","verified_run":"https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723","workflow":"English Pronunciation Coach TestFlight","xcode_version":"26.6"} -->

- Status: `fixed`
- Repository / workflow: `lrodeveloperr/English-pronunciation-` / `English Pronunciation Coach TestFlight`
- Xcode / stage: `26.6` / `upload`
- First observed: 2026-09-30T00:52:35+00:00
- Last updated: 2026-09-30T01:29:52+00:00
- Occurrences: 1
- Runner minutes wasted: 4.0
- Productivity minutes lost: 0.0
- Runs:
  - https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36651942464

**Symptom:** App Store Connect rejected build 1 with error 90035 because onnxruntime.framework/onnxruntime was not distribution-signed

**Root cause:** The unsigned archive left the embedded Swift-package framework without a signature; authenticated export did not repair a completely unsigned nested framework

**Fix:** Stage every embedded framework with a temporary ad-hoc signature before export, then require Apple Distribution authority and the mapped team on every exported framework

**Verified by:** https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723

**Prevention:** Recursively verify nested framework signatures before upload and reject ad-hoc or wrong-team signatures

**Notes:** None

## TF-DFDD211EE4 — auto-distributed-internal-group-rejects-redundant-assignment
<!-- testflight-issue-json: {"first_observed":"2026-09-30T01:29:52+00:00","fix":"Allow the idempotent 422 response, then verify group membership and the listing relationship authoritatively","id":"TF-DFDD211EE4","last_updated":"2026-09-30T01:29:52+00:00","minutes_wasted":1.0,"notes":"","prevention":"Treat assignment as idempotent only when a follow-up relationship read proves the exact build is present","productivity_minutes_lost":0.0,"repo":"lrodeveloperr/English-pronunciation-","root_cause":"App Store Connect auto-assigned the processed build to the internal group and rejects a redundant relationship POST for that group","runs":["https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36654215951"],"signature":"auto-distributed-internal-group-rejects-redundant-assignment","stage":"reporting","status":"fixed","symptom":"Delivery verification received HTTP 422 when it redundantly added a build already auto-distributed to the internal tester group","verified_run":"https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723","workflow":"English Pronunciation Coach TestFlight","xcode_version":"unknown"} -->

- Status: `fixed`
- Repository / workflow: `lrodeveloperr/English-pronunciation-` / `English Pronunciation Coach TestFlight`
- Xcode / stage: `unknown` / `reporting`
- First observed: 2026-09-30T01:29:52+00:00
- Last updated: 2026-09-30T01:29:52+00:00
- Occurrences: 1
- Runner minutes wasted: 1.0
- Productivity minutes lost: 0.0
- Runs:
  - https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36654215951

**Symptom:** Delivery verification received HTTP 422 when it redundantly added a build already auto-distributed to the internal tester group

**Root cause:** App Store Connect auto-assigned the processed build to the internal group and rejects a redundant relationship POST for that group

**Fix:** Allow the idempotent 422 response, then verify group membership and the listing relationship authoritatively

**Verified by:** https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723

**Prevention:** Treat assignment as idempotent only when a follow-up relationship read proves the exact build is present

**Notes:** None

## TF-F61FE490E0 — onnxruntime-framework-empty-minimum-os-version
<!-- testflight-issue-json: {"first_observed":"2026-09-30T01:29:52+00:00","fix":"Normalize each embedded framework MinimumOSVersion to the application minimum before signing, and verify the value survives the signed export","id":"TF-F61FE490E0","last_updated":"2026-09-30T01:29:52+00:00","minutes_wasted":14.0,"notes":"","prevention":"Check every embedded framework Info.plist for an application-compatible MinimumOSVersion before upload","productivity_minutes_lost":0.0,"repo":"lrodeveloperr/English-pronunciation-","root_cause":"The official ONNX Runtime Swift-package framework shipped with an empty or missing MinimumOSVersion value","runs":["https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36652595921"],"signature":"onnxruntime-framework-empty-minimum-os-version","stage":"processing","status":"fixed","symptom":"Apple processing rejected build 1 with error 90208 because onnxruntime.framework did not support the MinimumOSVersion recorded in its Info.plist","verified_run":"https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723","workflow":"English Pronunciation Coach TestFlight","xcode_version":"26.6"} -->

- Status: `fixed`
- Repository / workflow: `lrodeveloperr/English-pronunciation-` / `English Pronunciation Coach TestFlight`
- Xcode / stage: `26.6` / `processing`
- First observed: 2026-09-30T01:29:52+00:00
- Last updated: 2026-09-30T01:29:52+00:00
- Occurrences: 1
- Runner minutes wasted: 14.0
- Productivity minutes lost: 0.0
- Runs:
  - https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36652595921

**Symptom:** Apple processing rejected build 1 with error 90208 because onnxruntime.framework did not support the MinimumOSVersion recorded in its Info.plist

**Root cause:** The official ONNX Runtime Swift-package framework shipped with an empty or missing MinimumOSVersion value

**Fix:** Normalize each embedded framework MinimumOSVersion to the application minimum before signing, and verify the value survives the signed export

**Verified by:** https://github.com/lrodeveloperr/English-pronunciation-/actions/runs/36655305723

**Prevention:** Check every embedded framework Info.plist for an application-compatible MinimumOSVersion before upload

**Notes:** None

