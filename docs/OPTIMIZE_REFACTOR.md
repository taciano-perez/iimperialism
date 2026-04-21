# Optimization / Refactor Notes For Floppy Space

## Scope

This file tracks the remaining disk-space and memory-refactor options for the
Apple II floppy build. The goal is to reclaim real ProDOS blocks without changing
game behavior or shortening user-visible text.

This refresh is based on the current build as of `2026-04-15`, after the final
score was simplified to diplomacy, speed, and treasury, and after retaining only
the overflow guards that fit while keeping three free ProDOS blocks. The checks
used were:

- `make memory-usage`
- `java -jar tools/ac.jar -l assets/iimperialism.dsk`
- `od65 --dump-segments` on the current resident and overlay objects
- current sources in `src/`, `asm/`, `config/`, and `docs/`

This supersedes the older notes that still treated `MENU` as a large spare-space
overlay and `CNSL` as a possible merge target. Those assumptions are no longer
true in the current tree.

## Current State

Current disk catalog:

- `IIMPERIALISM` uses `68` ProDOS blocks and is `34,166` bytes long
- `IIMP.SYSTEM` uses `1` ProDOS block and is `469` bytes long
- `PRODOS` uses `34` ProDOS blocks
- there are `10` overlay files on disk:
  `ISCR`, `PSCR`, `TSCR`, `ASCR`, `DSCR`, `TXAC`, `BSCR`, `SSCR`, `MENU`, `CNSL`
- each overlay file is still padded to exactly `2,048` bytes and consumes
  `5` ProDOS blocks
- disk free space is `1,536` bytes, exactly the required three-block reserve

Current resident / overlay occupancy from `make memory-usage`:

### Resident

| Area | Used | Free |
|------|-----:|-----:|
| `LOWCODE` | 5306 | 724 |
| `RESIDENT_MAIN_SAFE` | 17198 | 1234 |
| `LANGUAGE_CARD` | 2635 | 437 |

High main pool breakdown:

| Segment | Size |
|---------|-----:|
| `CODE` | 8259 |
| `RODATA` | 8343 |
| `DATA` | 262 |
| `INIT` | 30 |
| `ONCE` | 304 |
| `BSS` | 264 |

Selected resident object contributions:

| Object | Notable contribution |
|--------|---------------------:|
| `main.o` | `2460` CODE, `601` RODATA, `187` BSS |
| `logic.o` | `4086` LOWCODE, `194` RODATA |
| `strings.o` | `184` CODE, `1225` RODATA |
| `trade_expedition.o` | `577` CODE, `67` RODATA |
| `ui.o` | `2635` LC, `4471` RODATA, `461` CODE |
| `text_hgr.o` | `516` LOWCODE, `176` RODATA |

### Overlays

| Overlay | UsedApprox | FreeApprox |
|--------|-----------:|-----------:|
| `ascr.bin` | 2043 | 5 |
| `bscr.bin` | 2018 | 30 |
| `cnsl.bin` | 2004 | 44 |
| `dscr.bin` | 1978 | 70 |
| `iscr.bin` | 1809 | 239 |
| `menu.bin` | 1836 | 212 |
| `pscr.bin` | 1512 | 536 |
| `sscr.bin` | 1665 | 383 |
| `tscr.bin` | 1960 | 88 |
| `txac.bin` | 2017 | 31 |

Important consequences:

- shrinking an overlay by 100 bytes does not reclaim floppy space while overlays
  are padded to `2048` bytes
- deleting one overlay file still reclaims `5` ProDOS blocks, or `2560` bytes
- shrinking `IIMPERIALISM` can reclaim space only when it crosses a ProDOS block
  boundary
- the current main binary is close to such a boundary:
  - `34,166` bytes means `67` data blocks plus `1` index block = `68` blocks
  - to drop to `67` total blocks, it must fit in `66` data blocks
  - `66 * 512 = 33,792`
  - target reduction is therefore about `374` bytes; use `400+` bytes as the
    practical target

## Fresh Conclusions

The remaining good options are more specific than "trim bytes somewhere."

The current project is constrained in three different ways:

1. The disk is tight, but not every byte saved becomes disk space.
2. Several overlays are still effectively full.
3. The main binary is close enough to a block boundary that a modest resident
   reduction is now valuable.

The most important change from the previous analysis is that resident guard
helpers are not currently affordable: they recover local overlay headroom, but
they spend a main-binary block and drop the disk below the required three-block
reserve. Future changes should either stay size-neutral or first recover at
least one ProDOS block.

## Recommended Options

### 1. Best near-term target: save one main-binary block

This is now the cleanest practical target.

Goal:

- reduce `IIMPERIALISM` by at least `400` bytes
- keep the file count unchanged
- confirm the catalog drops from `68` blocks to `67` blocks

Why this is attractive:

- the reduction needed is modest
- it avoids adding another overlay file
- it does not depend on finding a whole-screen merge
- resident headroom is currently acceptable:
  - `LOWCODE` has `724` bytes free
  - `RESIDENT_MAIN_SAFE` has `1234` bytes free
  - `LANGUAGE_CARD` has `437` bytes free

Likely places to inspect first:

- `src/main.c`: still owns splash, startup, nation naming, the main hub,
  resident helper exports, and battle prelude text
- `src/strings.c`: now holds diplomacy and final-report strings for constrained
  overlays; worthwhile to audit, but do not move strings back into `DSCR` or
  `CNSL` without measuring those overlays
- `src/logic.c`: still the largest LOWCODE object, though the score simplification
  already removed the obvious final-score dead weight
- `src/trade_expedition.c`: resident by design after deleting `TEXP`; moving it
  back into a new file would probably lose disk space, so only local shrinking is
  attractive
- UI picture/font RODATA in `ui.o`: large, but higher risk because it touches the
  shared drawing asset path

This should be preferred over a new overlay. A new overlay can shrink the main
binary while still losing disk space because the new file costs `5` blocks.

### 2. Strong disk-space candidate: variable-length overlays

Today every overlay binary is forced to exactly `2048` bytes by
`config/apple2-ovl.cfg`:

```text
OVL: file = %O, start = $8800, size = $0800, fillval = $00, fill = yes;
```

The loader also expects exactly `2048` bytes:

- `asm/prodos_overlay_load.s` reads up to `OVERLAY_SIZE`
- `src/overlay.c` treats any read count other than `OVERLAY_SIZE` as failure

That is simple and safe, but it means a small overlay still consumes the same
`5` ProDOS blocks as a nearly full one.

The current `PSCR` overlay uses about `1391` bytes. If overlays were stored
without padding, `PSCR` should fit below `1536` bytes, which would use:

- `3` data blocks
- `1` ProDOS index block
- `4` total blocks instead of `5`

That is a likely `512` byte disk recovery from `PSCR` alone.

Required implementation shape:

- stop padding overlay files in the overlay linker config
- clear the full `$8800-$8FFF` overlay slot before each read, or otherwise zero
  the unread tail after a short read
- remove or relax the exact `overlay_bytes_read == OVERLAY_SIZE` check
- preserve the maximum read count of `2048` bytes
- test every overlay path, because stale bytes in the overlay slot would be a
  serious failure mode

Why this is worth considering:

- it can reclaim disk space without deleting features
- it scales with future overlay shrinking
- it makes local overlay optimizations more meaningful

Why it is not entirely free:

- the loader needs a little more resident code
- the fixed-size read check currently catches malformed overlay files
- only overlays that cross a 512-byte ProDOS threshold reclaim actual blocks

Current expected immediate winner:

- `PSCR`: likely saves `1` block

Potential future winners if optimized below thresholds:

- overlays below `1536` bytes save `1` block
- overlays below `1024` bytes save `2` blocks, but no current nontrivial overlay
  is close to this except the menu C object alone, not the full `MENU` binary

### 3. Maintain overlay headroom before adding features

Several overlays are now too tight for comfortable feature work:

- `ASCR`: `5` bytes free
- `CNSL`: `44` bytes free
- `TXAC`: `31` bytes free
- `TSCR`: `88` bytes free
- `BSCR`: `30` bytes free
- `DSCR`: `70` bytes free

This does not directly free disk space while overlays are padded, but it is still
advisable maintenance. Without this, even small future behavior fixes can fail to
link.

Good local candidates:

- `ASCR`: repeated merchant/navy build prompt structure and similar resource
  deduction branches
- `TXAC`: trade action flow, likely branch consolidation and narrower locals
- `BSCR`: battle flow, especially repeated clear/print/sound sequences
- `TSCR`: repeated resource-row and transport-order selection logic
- `DSCR`: diplomacy proposal and trade-expedition setup flow
- `CNSL`: final report rendering and repeated string/number layout

Keep these optimizations local first. Moving more helper code into resident memory
should be reserved for cases where the overlay cannot be made to fit locally.

Measured but rejected because of the three-block floppy reserve:

- trade buy cap by remaining warehouse room
- saturating treasury gains on trade sells and battle bounty
- admiralty build caps for `traders` and `frigates`

### 4. Loader-owned splash remains plausible, but not first

The splash screen is still a resident-only startup cost:

- it is used once at boot
- it draws `SHIP_SPLASH`
- it participates in the timed `ESC` entropy path

A loader-owned splash could remove startup-only code and text from the main
binary without adding another overlay file. That remains a real option.

However, it is more fragile than the one-block resident cleanup or the
variable-length overlay idea because it crosses the loader/game boundary and must
preserve the current entropy handoff.

Recommended stance:

- keep it as a second-phase resident reduction
- do not create a dedicated splash overlay
- only pursue it after easier main-binary savings have been measured

### 5. ProDOS replacement is strategic, not near-term

Replacing ProDOS with RWTS or a custom disk path would free the largest amount of
space on paper because `PRODOS` uses `34` blocks.

It is not a tactical optimization:

- overlay loading depends on ProDOS MLI today
- save/load depends on ProDOS MLI today
- the boot path and file layout would change substantially
- validation cost would be much higher than code-size tuning

Keep this as a long-term distribution/bootloader project, not the next refactor.

## Options To Avoid For Now

### Do not chase another overlay merge first

A whole-overlay deletion would still be the biggest single win, but the current
pairing options are poor.

Why:

- `MENU` has only `212` bytes free once save/load support and
  `prodos_gamestate_io.o` are linked into it
- `CNSL` has only `44` bytes free and owns the full endgame flow
- `PSCR` has room, but no current overlay is small enough to fit inside it
- the tight overlays would need major rewrites before they could absorb or be
  absorbed by another screen

Conclusion:

- do not plan around deleting another overlay until the overlay set has been
  deliberately reshaped

### Do not move the main screen wholesale into an overlay

`src/main.c` is not just the main screen.

It also owns:

- startup and splash flow
- nation-name prompt
- resident screen dispatch
- helper exports used by overlays, including `render_warehouse_box()` and
  `render_turn_funds_header()`
- the battle prelude
- main-screen rating helpers

The current object is large (`2460` CODE plus `601` RODATA), but extracting a
hub-screen overlay would require disentangling several resident responsibilities
first. That may become worthwhile later, but it is not a clean disk-space move
today.

### Do not create a new trade-market overlay

The former `TEXP` overlay was removed because keeping the small market renderer
resident and retaining only `TXAC` as an overlay recovered net disk space.

Bringing that screen back as a separate file would add another `5`-block overlay
cost. It should not be reconsidered unless multiple trade-related screens are
being merged into a different file-count strategy.

### Do not move final-report strings back into `CNSL`

`CNSL` has only `44` bytes free. The resident final-report string/helper split is
still the right tradeoff for keeping the endgame report inside the existing
Council overlay.

## Recommended Order

1. Try to save `400+` bytes from the main binary without adding files.
2. If that succeeds, verify `IIMPERIALISM` drops from `68` to `67` blocks and
   disk free space rises above the three-block reserve.
3. Prototype variable-length overlays and confirm whether `PSCR` drops from `5`
   to `4` blocks.
4. Add local headroom to the tight overlays, especially `ASCR`, `TXAC`, `CNSL`,
   `DSCR`, `TSCR`, and `BSCR`.
5. Revisit loader-owned splash only after the easier block-boundary work has been
   measured.
6. Treat ProDOS replacement as a separate long-term project.

## Validation Checklist

After each candidate change:

1. run `make memory-usage`
2. run `java -jar tools/ac.jar -l assets/iimperialism.dsk`
3. check `IIMPERIALISM` block count and byte length
4. check every overlay's used/free bytes
5. check whether any overlay file actually disappeared or crossed a 512-byte
   ProDOS block threshold

Behavior checks:

- boot still works from the disk image
- splash still preserves timed-`ESC` entropy if touched
- every overlay loads and returns correctly
- production, transport, admiralty, diplomacy, trade expedition, battle, science,
  menu, save/load, and Council report flows still work
- if variable-length overlays are used, repeated overlay loads must not show stale
  graphics, text, state, or code behavior from a previous larger overlay
