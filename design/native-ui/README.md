# Native UI mockup

`mockup.html` is a standalone page (open it in any browser; the Noto Sans JP web font is optional) showing the
native iOS 18 UI on three layouts at one shared point scale:

- iPhone 16, 393 × 852 pt
- iPad one column (mini / 11" portrait), 744 × 1133 pt
- iPad two columns (11" landscape), 1210 × 834 pt

Screens: 冒険, ステージ, 録音, 結果, 図鑑, 進捗 and the Pro（購読）sheet, in light and dark. The 使用部品ラベル switch tags
each block with the SwiftUI component it maps to. Icons are SVG stand-ins for SF Symbols.

The shipped SwiftUI implementation and how it differs from this mockup are described in
[`docs/native-ui.md`](../../docs/native-ui.md). This replaces the creature-based concept in `../ui-mockup/`.
