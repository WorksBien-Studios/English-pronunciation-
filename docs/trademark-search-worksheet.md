# Trademark search — status and worksheet

**Date:** 2026-09-29  
**Status:** `NOT CLEARED` — the official trademark databases could not be queried from the build environment.  
**Not legal advice.** Have a 弁理士 (registered patent attorney) confirm before commercial use of any character name as a brand.

## What was attempted

| Source | Result |
|---|---|
| J-PlatPat (JPO) | Blocked by the environment's network egress proxy; also a session-driven JavaScript search app that cannot be queried by URL. |
| WIPO Global Brand Database | Blocked by the egress proxy. |
| USPTO, EUIPO eSearch | Not attempted: interactive JavaScript search tools with the same limitation. |
| General web searches for each name (+ 商標 / キャラクター) | Done, see below. |

Web search cannot see unregistered-in-the-news marks, and pending or registered marks for obscure goods do not surface. **No hits on the web is not evidence that a name is free.**

## Preliminary web findings (names as of PR #7)

| Name | Sound (katakana) | Finding |
|---|---|---|
| くるん | クルン | No specific character or trademark surfaced. |
| ぺろ | ペロ | No specific character surfaced; very short, common onomatopoeia. |
| べー | ベー | No specific character surfaced; extremely short and common. |
| はむ | ハム | Near とっとこハム太郎 and ordinary "ham/hamster" terms; different word, but close in sound family. |
| ふーふー | フーフー | No specific character surfaced. |
| ぷっぷ | プップ | No exact match; Sanrio's ポムポムプリン and similar names surfaced in the same space (different names). |

**Expectation:** very short onomatopoeic names (ペロ, ベー, ハム, プップ) are the most likely to collide with existing marks in classes 9, 28 and 41, and are also weak as trademarks (low distinctiveness). Treat them as placeholders.

## Search to run (J-PlatPat → 商標 → 商標検索)

Search each phonetic string (称呼) with a similar-sound search (称呼検索・類似), in these classes:

- **9** — downloadable software / apps  
- **41** — education, instruction, entertainment, games  
- **28** — games, toys, plush  
- **16** — printed matter, stickers  
- **25** — apparel (only if merchandise is planned)

| Item | Search strings (称呼) | Also search |
|---|---|---|
| くるん | クルン | クルクル, クルリン (near forms) |
| ぺろ | ペロ | ペロリ, ペロペロ |
| べー | ベー | — |
| はむ | ハム | ハムハム, ハム太郎 (to see the neighbouring rights) |
| ふーふー | フーフー | フウフウ |
| ぷっぷ | プップ | プッパ, プープ |
| App name | エイゴハツオンコーチ | 英語発音コーチ (text search), 発音矯正 |
| Concept | オトノシマ | 音の島 |
| Guide character | (unnamed — choose a name first) | — |

Figurative (character artwork) marks cannot be cleared by text search: ask a 弁理士 to run a 図形等分類 (Vienna-code) search on the final artwork.

Repeat the phonetic search in the USPTO and EUIPO databases only if the app is distributed outside Japan.

## Results (fill in)

| Item | Class 9 | Class 41 | Class 28 | Conflicting mark(s) / owner | Decision (keep / rename) |
|---|---|---|---|---|---|
| くるん | | | | | |
| ぺろ | | | | | |
| べー | | | | | |
| はむ | | | | | |
| ふーふー | | | | | |
| ぷっぷ | | | | | |
| 英語発音コーチ (app name) | | | | | |
| 音の島 | | | | | |

## Decision rules

1. Identical or similar-sounding mark by another party in class 9 or 41 (or 28 for merchandise) for the same or similar goods/services → **rename**.
2. If only remote classes conflict → keep, record it, and re-check before any merchandise.
3. If every candidate name is short and generic, prefer longer, more distinctive names; distinctiveness helps both clearance and any later registration.
4. Whichever names survive, do not put them in the App Store name, subtitle or keywords, and do not describe the app as "monsters" (see [`character-similarity-screen.md`](character-similarity-screen.md)).
