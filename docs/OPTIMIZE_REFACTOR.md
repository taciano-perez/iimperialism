# Optimization / Refactor Notes For Floppy Space

## Scope

This file is the current disk-space analysis for the Apple II floppy build.
The goal is to reclaim real ProDOS blocks without changing game behavior or
shortening user-visible text.

This refresh is based on the current build as of `2026-04-12`, using:

- `make memory-usage`
- `java -jar tools/ac.jar -l assets/iimperialism.dsk`
- `build/iimperialism.map`
- current sources in `src/`, `asm/`, and `docs/`

This supersedes the earlier overlay-merge notes. The `TEXP` market overlay has
now been removed: its small renderer lives in resident code, while the larger
trade action flow remains in `TXAC`.

## Current State

Current disk catalog:

- `IIMPERIALISM` uses `68` ProDOS blocks
- `IIMP.SYSTEM` uses `1` ProDOS block
- `PRODOS` uses `34` ProDOS blocks
- there are now `10` overlay files on disk:
  `ISCR`, `PSCR`, `TSCR`, `ASCR`, `DSCR`, `TXAC`, `BSCR`, `SSCR`, `MENU`, `CNSL`
- each overlay file still consumes `5` ProDOS blocks
- disk free space is now `1536 bytes`

Current resident / overlay occupancy from `make memory-usage`:

### Resident

| Area | Used | Free |
|------|-----:|-----:|
| `LOWCODE` | 5402 | 628 |
| `RESIDENT_MAIN_SAFE` | 16915 | 1517 |
| `LANGUAGE_CARD` | 2593 | 479 |

### Overlays

| Overlay | UsedApprox | FreeApprox |
|--------|-----------:|-----------:|
| `ascr.bin` | 1659 | 389 |
| `bscr.bin` | 1992 | 56 |
| `cnsl.bin` | 1765 | 283 |
| `dscr.bin` | 1978 | 70 |
| `iscr.bin` | 1804 | 244 |
| `menu.bin` | 917 | 1131 |
| `pscr.bin` | 1391 | 657 |
| `sscr.bin` | 1665 | 383 |
| `tscr.bin` | 1938 | 110 |
| `txac.bin` | 2003 | 45 |

Important consequences:

- shrinking an overlay by 100 bytes does not reclaim floppy space by itself
- floppy space is reclaimed only if:
  - the main binary drops enough to free one or more ProDOS blocks, or
  - two overlays are merged so one 2 KB overlay file disappears
- moving the former `TEXP` renderer into resident code grew the main binary from
  `66` to `68` ProDOS blocks, but deleting the `TEXP` file removed a `5`-block
  overlay, for a net recovery of `3` blocks
- the Council victory report has been implemented inside the existing `CNSL`
  overlay
- the victory report did not add a new overlay file or picture asset
- it did consume Council-overlay headroom and added a small resident
  helper/string surface

## Fresh Conclusions

The best current options are no longer just "trim bytes somewhere." The project
has already taken the small-screen resident move for `TEXP`, so remaining work
falls into two separate buckets:

### A. Reclaim another whole overlay file

This is still the cleanest immediate disk-space win.

One successful merge removes one `5`-block overlay file from disk. That is
`2560` bytes of real floppy space.

### B. Shrink the resident main binary without adding files

This matters when the resident main binary is sitting on the wrong side of a
ProDOS block boundary, which it currently is.

This is different from creating a brand-new overlay:

- moving code into a new overlay can reduce the main binary but still lose on
  disk because the new file also costs `5` blocks
- resident-only reductions or moves into existing files can reduce the main
  binary and avoid any new disk-file cost

## Overlay Merge Candidates

The previous best candidate, `MENU + TEXP`, is no longer available because
`TEXP` has been deleted rather than merged. `MENU` still has substantial
headroom, but it is intentionally left unmerged so it can grow its save/load and
menu flow later.

### Pairings involving `CNSL`

`CNSL` is no longer a good merge candidate.

Reason:

- it now owns both the Council vote table and the final victory report
- the victory report deliberately uses resident strings and helpers to stay under
  the 2 KB overlay limit
- future endgame polish is likely to need the remaining `CNSL` headroom

Do not plan on merging another screen into `CNSL` unless the victory report is
substantially rewritten or more space is reclaimed first.

## Resident Strategy

The resident-side result that matters now is simple:

- `IIMPERIALISM` is now `68` blocks after absorbing the trade market renderer
- `ISCR` is up to about `1804` bytes used
- `TEXP` is gone, so future resident work should not assume it can be merged
  with another overlay

That means future resident work should focus on general resident reductions, not
on undoing or re-litigating the completed industry/ledger refactor.

### Main screen: not the first resident screen to move

The user specifically called out the main screen. I do not think it is the best
first move.

Why:

- `src/main.c` does much more than render the main screen
- it also owns startup flow, splash timing, nation naming, the resident screen
  dispatch loop, helper exports like `render_warehouse_box()` and
  `render_turn_funds_header()`, and the resident battle prelude
- several small rating helpers are tightly coupled to `render_main_screen()`
- the current `main.o` contribution is already large:
  - `CODE` = `0x0990` = `2448` bytes
  - `RODATA` = `0x0258` = `600` bytes

That does not mean the whole main screen is bigger than 2 KB by itself, but it
does mean "move main.c into an overlay" is not a real option.

A practical main-screen extraction would first need to split:

- splash / startup work
- nation-name prompt
- generic resident helpers needed by overlays
- resident dispatch loop
- battle prelude

Only after that split would it be clear whether the remaining hub screen is
small enough and cohesive enough to live in an overlay.

Current recommendation:

- do not start with the main screen
- revisit the main screen only after splash and startup responsibilities are
  disentangled

## Splash Screen Reassessment

The earlier document was correct that the splash is still one of the largest
resident-only feature costs, because `SHIP_SPLASH` is only used once at boot.

That remains true, but the disk-space context has changed:

- adding a new splash overlay right now would cost another `5` blocks on a disk
  with only `512 bytes free`
- so a standalone splash overlay is no longer a good first move for floppy space

Re-ranked options:

### 1. Best disk-aware option: loader-owned splash

Move splash rendering and timed `ESC` capture into `IIMP.SYSTEM`, then pass the
measured entropy into the main program before launch.

Why it now ranks first:

- removes the splash from the main binary
- does not add a new overlay file
- preserves the current entropy model if the handoff is done carefully

Cost:

- loader work is more fragile than normal game-code work

### 2. Best non-loader option: piggyback splash on an existing overlay file

If splash logic is moved out of resident code, do not create a twelfth overlay
file just for it.

Instead:

- piggyback the splash onto some existing low-occupancy overlay file, with a
  dispatcher
- or merge splash work into some startup-only path that does not increase file
  count

This is more awkward than a dedicated splash overlay, but it respects the
actual disk constraint.

### 3. Dedicated splash overlay

This is now lower priority.

It still helps resident size, but it is a poor disk-space move while the floppy
is already full.

## Lower-Value Or Deferred Resident Work

These remain valid, but they are not the first thing I would do under the
current low-free-space pressure.

### Replace the 192-entry `HGR_ROWS[]` table

Still plausible resident win:

- about `300+` bytes feels realistic

But today it is behind:

- one-file overlay merge
- splash relocation that does not add a file

### Revisit resident-only strings

This has already produced useful savings and is still worth asking:

- are any resident strings only serving one overlay that already has room

But this is now a secondary tuning pass, not the primary strategy. One current
exception is the victory report: its strings intentionally remain resident because
`CNSL` is the constrained side of the tradeoff.

### Bold font derivation / picture compression

Still potentially useful, but still higher risk than the options above.

## Recommended Order

There are now two sensible paths, depending on whether the first priority is
"free the most blocks quickly" or "keep shrinking resident further."

### Path A: fastest disk recovery

1. find another whole overlay file that can be deleted without consuming planned
   `MENU` headroom
2. re-run `make memory-usage` and disk catalog
3. if more space is still needed, revisit loader-owned splash or another
   no-new-file resident reduction
4. revisit splash relocation without adding a new disk file

Why this path ranks first:

- deleting one more overlay would free `5` blocks immediately
- it is the clearest way off a completely full floppy

### Path B: resident-first cleanup

1. verify future resident changes against the current `68`-block main binary
2. pursue general resident reductions that do not add a new disk file
3. if more space is still needed, identify a new overlay deletion candidate
4. then revisit splash relocation without adding a new disk file

Why this path is attractive:

- it avoids adding files or consuming planned `MENU` headroom
- it keeps future work focused on net wins rather than revisiting completed refactors

Why I still rank it second:

- a guaranteed `5`-block overlay deletion is still more valuable than another likely
  one-block resident drop

## Final Recommendation

If I had to pick the best next move today, it would be:

1. keep the implemented ledger move in `ISCR`
2. keep `MENU` available for planned menu growth
3. revisit loader-owned splash if resident size still needs to come down

The main screen can be reconsidered later, but only after startup / splash /
dispatch responsibilities are peeled away from `src/main.c`.

## Validation Checklist

After each candidate change:

1. run `make memory-usage`
2. run `java -jar tools/ac.jar -l assets/iimperialism.dsk`
3. check `IIMPERIALISM` block count
4. check whether an overlay file actually disappeared from the disk catalog

Also verify behavior:

- boot still works
- splash still preserves timed-`ESC` entropy if touched
- industry can still open and close ledger correctly
- resident trade expedition market rendering still flows into `TXAC`
- save/load still works if `MENU` is touched
