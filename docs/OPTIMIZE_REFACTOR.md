# Optimization / Refactor Notes For Floppy Space

## Scope

This file summarizes ideas to reduce code and disk footprint without changing game behavior or shortening any user-facing strings.

I reviewed:

- `README.md`
- `docs/FLOPPY.md`
- `docs/MEMORY.md`
- `docs/OPTIMIZE_CODE.md`
- `docs/PICTURES.md`
- `docs/FONT.md`
- `Makefile`
- the current linker map and current disk catalog
- the older overlay-only analysis that previously lived in `docs/OV_MERGE_REFACTOR.md`

This document is now the canonical place for both general size work and overlay-merge analysis.

## Current State

Current disk catalog after implementing options 1, 5, and 6 below:

- `IIMPERIALISM` uses `64` ProDOS blocks
- each overlay file uses `5` ProDOS blocks
- disk free space is `1024 bytes`

Current overlay occupancy from `make memory-usage`:

| Overlay | UsedApprox | FreeApprox |
|--------|-----------:|-----------:|
| `ascr.bin` | 1660 | 388 |
| `bscr.bin` | 1992 | 56 |
| `cnsl.bin` | 1164 | 884 |
| `dscr.bin` | 1982 | 66 |
| `iscr.bin` | 944 | 1104 |
| `menu.bin` | 917 | 1131 |
| `pscr.bin` | 1391 | 657 |
| `sscr.bin` | 1523 | 525 |
| `texp.bin` | 835 | 1213 |
| `tscr.bin` | 1938 | 110 |
| `txac.bin` | 1973 | 75 |

Important consequence:

- shrinking an overlay by 100 bytes does not reclaim floppy space by itself
- floppy space is reclaimed only if:
  - the main binary drops enough to free ProDOS blocks, or
  - two overlays are merged so one 2 KB overlay file disappears

## Highest Value Options

### 1. Remove unused resident picture assets

Status: implemented

This is the cleanest resident-space win I found.

The following picture data appeared to be compiled into the main binary but not referenced anywhere in current game code:

- `COUNTRY1_DATA`
- `COUNTRY2_DATA`
- `COUNTRY3_DATA`
- `COMPASS_DATA`

These were defined in `include/pictures.h` and exported through `PICTURES_DATA[]`, but had no actual use sites.

Approximate resident data tied up in those four unused assets:

- `COUNTRY1_DATA`: about `114` bytes
- `COUNTRY2_DATA`: about `114` bytes
- `COUNTRY3_DATA`: about `226` bytes
- `COMPASS_DATA`: about `114` bytes
- pointer-table entries: about `8` more bytes

Total: about `576` bytes

Why this mattered:

- this saved real bytes in the main binary
- unlike overlay-local shaving, this could directly reduce ProDOS block usage
- it also reduced resident `RODATA`, which is already large

Actual result after implementation:

- `IIMPERIALISM` dropped from `32,885` bytes to `32,317` bytes
- ProDOS allocation for `IIMPERIALISM` dropped from `66` blocks to `65` blocks
- disk free space increased from `0 bytes` to `512 bytes`

Risk:

- low

### 2. Split the splash-only ship art out of the resident binary

Status: deferred for now

`SHIP_SPLASH_DATA` is very large and is only used once during startup in `src/main.c`.

Its header says:

- width = `0x1a` = `26` bytes
- height = `0x43` = `67` rows

So its raw embedded payload is about:

- `26 * 67 + 2 = 1744` bytes

That is a major share of resident `RODATA` for one startup-only image.

Critical constraint:

- the splash screen is part of the current entropy path
- the game currently seeds randomness from the human-timed delay before `ESC`
- any splash refactor must preserve the equivalent of `seed_random(wait_for_splash_escape())`
- moving the art is acceptable; losing the timed wait is not

Best options:

#### 2.1 Startup overlay after `ui_init()`

Move the splash drawing code and splash text to a dedicated overlay, but keep the actual `wait_for_splash_escape()` and `seed_random()` call resident in `src/main.c`.

Suggested flow:

1. boot into main binary
2. `ui_init()`
3. `run_overlay(OVL_SPLASH)` draws the splash and returns
4. resident `seed_random(wait_for_splash_escape())`
5. continue to `start_new_game()`

Why this ranks first:

- preserves the current RNG behavior exactly
- removes `SHIP_SPLASH_DATA` from resident `RODATA`
- uses the already-established overlay machinery
- startup logic stays understandable

Tradeoff:

- adds another overlay file unless it is merged with some other small startup-only screen later
- that means this is mainly a resident-memory win unless paired with another disk-space change

#### 2.2 Loader-owned splash in `IIMP.SYSTEM`

Move splash rendering and the timed `ESC` wait into `IIMP.SYSTEM`, then pass the measured entropy into the main program through a fixed memory location before launching `IIMPERIALISM`.

Why it is attractive:

- removes the splash art from the main binary completely
- does not require a new overlay file on disk

Why it ranks second:

- requires custom startup graphics work in the loader
- requires a defined handoff contract from loader to main program for the entropy value
- loader code is a more fragile place to iterate than normal resident/overlay code

This is the best disk-structure option, but not the safest implementation option.

#### 2.3 Separate raw splash asset loaded by main

Store the splash art as its own disk asset and have resident startup code load it before calling `wait_for_splash_escape()`.

Why it works:

- preserves the RNG seeding behavior
- removes the splash art from the main binary

Why it ranks lower:

- adds another file to the floppy
- needs either a one-off asset-loading path or a generalization of the current overlay loader
- weaker disk-space story than the loader option

#### 2.4 Compress the splash art but keep it resident

Keep splash logic in `src/main.c`, but store `SHIP_SPLASH_DATA` in a compressed format and decompress it only for startup rendering.

Why it ranks lowest:

- it saves resident bytes, but usually less than moving the art out entirely
- it adds decode code back into the main binary
- it does not improve file count or disk structure

Recommendation:

- safest refactor: `2.1` startup overlay, with resident `wait_for_splash_escape()`
- strongest disk-structure refactor: `2.2` loader-owned splash with entropy handoff

Expected gain:

- roughly `1.7 KB` of resident `RODATA`
- likely enough to reclaim multiple ProDOS blocks from `IIMPERIALISM` if implemented cleanly

### 3. Replace the 192-entry `HGR_ROWS[]` table in `include/pictures.h`

Status: deferred for now

`draw_picture()` currently uses a full 192-entry HGR row-address table:

- `192 * 2 = 384` bytes of `RODATA`

But all actual call sites go through `draw_picture_at()` in `src/ui.c`, which converts a character row to `y * 8`.

That means picture drawing is currently character-row aligned in practice.

There is already a smaller 24-row base table in `asm/text_hgr.s`:

- `CHAR_ROW_BASE_LO`
- `CHAR_ROW_BASE_HI`

This suggests a smaller picture-row addressing strategy is possible:

- compute from a 24-entry char-row base
- then advance row addresses inside the picture loop
- or share a compact row-base helper

Expected gain:

- likely a few hundred bytes resident
- roughly `300+` bytes looks realistic even after adding some code

Risk:

- low to medium
- safe if picture drawing remains character-row aligned
- higher risk only if arbitrary pixel `y` placement is needed later

### 4. Merge one overlay pair and delete one file from disk

Status: deferred for now

This is the most direct way to reclaim floppy space.

One merged overlay removes one overlay file entirely, which should save one overlay file allocation on disk. Today that means roughly `5` ProDOS blocks for each successful merge.

Current ABI constraint:

- `run_overlay()` loads one file and always jumps to `$8800`
- a merged overlay therefore needs a dispatcher at `$8800`
- separate overlay IDs can still exist, but they cannot jump to distinct offsets directly under the current ABI

Relevant files:

- `src/overlay.c`
- `include/overlay.h`
- `config/apple2-ovl.cfg`
- `docs/MEMORY.md`

#### Best current candidate: `ISCR + TEXP`

Current approximate usage:

- `iscr.bin` = `944`
- `texp.bin` = `835`
- combined = `1779`

That leaves about `269` bytes before the 2 KB limit, which is usable headroom for a small dispatcher.

Why this pair is better than it first appears:

- both screens already use `render_turn_funds_header()`
- both use `render_warehouse_box()`
- both draw `INDUSTRY_PORTRAIT`
- dispatch can be based on `state.current_screen`
- no new selector variable is required

Why this matters:

- this is the single clearest disk-space win available without touching the main binary

Tradeoff:

- the pairing is not architecturally ideal
- it is a disk-saving refactor first, not a cleanliness refactor

#### Secondary candidate: `MENU + TEXP`

Approximate combined usage:

- `917 + 835 = 1752`

This fits comfortably by size, but it needs an explicit selector because `MENU` is not uniquely inferable from `state.current_screen`.

I would rank it below `ISCR + TEXP`.

#### Candidate not currently viable anymore

The older analysis favored:

- `industry + transport`
- `industry + production`
- `transport + production`

Those no longer fit with the current build:

- `iscr + tscr = 2882`
- `iscr + pscr = 2335`
- `tscr + pscr = 3329`

So those rankings are historically useful, but not current.

#### Historical merge analysis from 2026-03-18

From the older `make memory-usage` snapshot on 2026-03-18:

| Overlay | UsedApprox | UsedPercent |
|---------|------------|-------------|
| `iscr.bin` | 783 | 38.23% |
| `pscr.bin` | 988 | 48.24% |
| `tscr.bin` | 842 | 41.11% |
| `menu.bin` | 915 | 44.68% |
| `texp.bin` | 809 | 39.50% |
| `dscr.bin` | 1190 | 58.11% |
| `bscr.bin` | 1425 | 69.58% |
| `sscr.bin` | 1553 | 75.83% |
| `ascr.bin` | 1695 | 82.76% |
| `txac.bin` | 1939 | 94.68% |

That earlier ranking was:

1. `industry + transport`
2. `industry + production`
3. `transport + production`
4. `diplomacy + trade market`

Useful historical takeaways that still apply:

- the `industry` / `transport` / `production` family is the most natural merge area when sizes permit
- `diplomacy + trade market` is cohesive, but tends to be fragile on size
- any pairing involving `menu` is awkward because `menu` is not selected only by `state.current_screen`
- `trade market + trade action` is the most natural semantic pair, but it is far too large to fit under the current 2 KB overlay limit

Suggested merge strategy if this is revisited:

1. keep separate overlay IDs if that helps resident call sites stay simple
2. map multiple IDs to one on-disk file if needed
3. place one assembly dispatcher stub at `$8800`
4. dispatch from `state.current_screen` whenever possible
5. re-run `make memory-usage` after linking the merged image, because `.bin` occupancy is what matters, not just source intuition

Important non-goal:

- overlay merging does not reduce runtime RAM usage
- it only reduces disk footprint and overlay file count
- the execution window at `$8800-$8FFF` remains 2 KB either way

## Strong Secondary Options

### 5. Remove the `atoi()` dependency from `scan_uint()`

Status: implemented

`scan_uint()` in `src/ui.c` currently built a text buffer and then called `atoi()`.

The map showed that this pulled in at least:

- `atoi.o`
- `ctype.o`
- `ctypemask.o`

That was a few hundred bytes of main-binary code/rodata for one very narrow use case.

A custom digit accumulator inside `scan_uint()` preserved behavior while avoiding those library pulls.

Expected gain:

- likely a few hundred resident bytes

Actual result after implementation:

- `IIMPERIALISM` dropped from `32,317` bytes to `32,084` bytes
- ProDOS allocation for `IIMPERIALISM` dropped from `65` blocks to `64` blocks
- disk free space increased from `512 bytes` to `1024 bytes`

Risk:

- low
- easy to test

### 6. Replace `strcmp()` / `strcpy()` uses in name assignment with fixed-size helpers

Status: implemented

`logic.c` used `strcmp()` and `strcpy()` for short, bounded nation names.

Because these names are fixed-length and very short, custom bounded copy/compare helpers may let the main binary avoid:

- `strcmp.o`
- `strcpy.o`

This is not a huge win by itself, but it stacked well with the `atoi()` removal.

Expected gain:

- modest

Actual result after implementation:

- resident code now uses local bounded helpers instead of `strcmp()` / `strcpy()` for nation-name initialization and startup name copy
- `IIMPERIALISM` stayed at `64` ProDOS blocks
- disk free space stayed at `1024 bytes`

Conclusion:

- this was behavior-safe cleanup
- it improved control over resident dependencies
- it did not reclaim additional floppy space in the current build

Risk:

- low

### 7. Revisit which strings really need to stay resident

`strings.o` itself is not the main problem, but the main binary has a lot of resident `RODATA` overall.

The current project already moved diplomacy strings into resident storage to shrink `dscr.bin`. That was the right choice for overlay pressure.

For floppy pressure, the next useful question is narrower:

- are there any resident strings that are only used by one small overlay and could move back into that overlay without blocking a merge candidate?

This is not a blanket recommendation. It only makes sense if it helps one of these:

- reduce main binary block count
- or make a specific overlay merge feasible

## Lower Priority / Situational Ideas

### 8. Generate bold text from the regular font instead of storing a second full font table

`font_data` and `font_bold_data` together cost:

- `96 * 8 * 2 = 1536` bytes

If bold rendering could be derived from the regular font at render time, the project could potentially remove most or all of `font_bold_data`.

This is attractive in pure size terms, but I consider it higher risk because:

- visual output must remain acceptable
- runtime code gets more complex
- exact bold glyph appearance may change

I would not do this before the lower-risk resident wins above.

### 9. Compress picture data

Picture data is another major resident `RODATA` consumer.

Options:

- simple row RLE
- store repeated blank rows compactly
- pack only body rows and reconstruct empty margins

This could save meaningful bytes, especially on sparse images, but the tradeoff is extra decode code in resident memory.

I would only explore this after:

- removing unused art
- splitting splash art
- shrinking the picture row table

### 10. Replace `draw_picture()` generality with the simpler contract the game actually uses

Right now the resident picture path is more general than the current callers need.

Observed current usage:

- all callers use `draw_picture_at()`
- all calls are grid-aligned
- picture set is fixed and known

If that contract is accepted explicitly, the picture renderer can probably be specialized further:

- fewer general-case calculations
- possibly smaller metadata
- tighter loop code

This is worth considering, but it is more invasive than the table reduction above.

## Ideas That Do Not Help Floppy Space Much

### Overlay-local micro-optimizations by themselves

Examples:

- shrinking `txac.bin` by 100 bytes
- shaving 50 bytes from `bscr.bin`

These are still worth doing when an overlay is close to 2 KB, but they do not reclaim disk blocks unless they enable an actual overlay merge.

### `include/icons.h`

`include/icons.h` appears unused, but since nothing includes it, it is not contributing to the built binaries. Cleaning it up may help repo hygiene, but it does not help the floppy.

### Removing unused declarations from `include/strings.h`

There are stale declarations there, but declarations alone do not materially affect disk footprint.

## Recommended Order

If the goal is to get disk space back with the best risk/reward ratio, I would do the remaining work in this order:

1. replace the 192-entry `HGR_ROWS[]` table with a smaller strategy compatible with `draw_picture_at()`
2. revisit the deferred items:
   `SHIP_SPLASH_DATA` split-out, picture/overlay disk strategy, and overlay merge `ISCR + TEXP`

## Expected Payoff

The combination of these resident wins:

- unused pictures: about `576` bytes
- picture row table reduction: about `300+` bytes
- `atoi()` and related library pulls: implemented, saved one additional ProDOS block
- fixed-size name helpers: implemented, but no additional disk-block win in the current build

should be enough to make the main binary materially smaller.

And separately:

- merging `ISCR + TEXP` should reclaim one overlay file allocation on disk

That combined plan is the most realistic path I see to getting the floppy back off `0 bytes free` without changing behavior or shortening strings.

## Validation Notes

After each candidate change:

1. run `make`
2. run `make memory-usage`
3. run `java -jar tools/ac.jar -l assets/iimperialism.dsk`

For disk work, AppleCommander block counts matter more than raw host-file byte counts.

For resident-data changes, also verify:

- boot still works
- overlays still load correctly
- splash still renders correctly if startup assets are touched
