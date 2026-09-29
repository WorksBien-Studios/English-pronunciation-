# English Pronunciation Coach — App Store Screenshot Shells

Editable Japanese App Store screenshot-shell masters are provided for **both supported device classes**.

## Device sets

### iPhone — 1320 × 2868

1. **01-diagnosis.svg** — 苦手な音が、すぐわかる
2. **02-correction.svg** — 直し方が、日本語でわかる
3. **03-progress.svg** — 発音を練習して、すぐ再チェック

Replacement slot: x=116, y=692, width=1088, height=1980, radius=78.

### iPad — 2064 × 2752

**Capture requirement: use the real two-column iPad `NavigationSplitView` state. Do not use the compact/single-column iPhone-like state for App Store screenshots.**

1. **ipad-01-diagnosis.svg** — 苦手な音が、すぐわかる  
   Left: diagnosed weak sounds / recommended order. Right: selected diagnosis/result detail.
2. **ipad-02-correction.svg** — 直し方が、日本語でわかる  
   Left: lesson / target-sound list. Right: selected correction lesson with IPA and articulatory guidance.
3. **ipad-03-progress.svg** — 発音を練習して、すぐ再チェック  
   Left: practice/review context. Right: recording result, feedback, and retry loop.

Replacement slot: x=245, y=610, width=1575, height=2100, radius=70. The slot stays one rectangle because it receives one full authentic iPad screenshot; the **two-column structure must be inside that capture**.

## Rule

The large rounded rectangle is a **replacement slot**, not mock app UI. Insert the authentic release-build capture for the matching device class into that slot during the screenshot run. Do not reuse an iPhone capture inside the iPad shell or vice versa. Keep the headline/caption layer unchanged.

The exact device canvas sizes, replacement coordinates, filenames, and captions are machine-readable in **shell-spec.json**.

The SVG masters intentionally use system Japanese fonts and contain no embedded font files.
