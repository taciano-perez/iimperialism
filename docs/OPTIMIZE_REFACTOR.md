# Optimization / Refactor Notes For Floppy Space

## Scope

This file is the current disk-space analysis for the Apple II floppy build.
The goal is to reclaim real ProDOS blocks without changing game behavior or
shortening user-visible text.

This refresh is based on the current build as of `2026-04-06`, using:

- `make memory-usage`
- `java -jar tools/ac.jar -l assets/iimperialism.dsk`
- `build/iimperialism.map`
- current sources in `src/`, `asm/`, and `docs/`

This supersedes the earlier overlay-merge notes and the older
`0 bytes free -> 1024 bytes free` snapshot. The disk is full again.

## Current State

Current disk catalog:

- `IIMPERIALISM` uses `66` ProDOS blocks
- `IIMP.SYSTEM` uses `1` ProDOS block
- `PRODOS` uses `34` ProDOS blocks
- there are now `11` overlay files on disk:
  `ISCR`, `PSCR`, `TSCR`, `ASCR`, `DSCR`, `TEXP`, `TXAC`, `BSCR`, `SSCR`, `MENU`, `CNSL`
- each overlay file still consumes `5` ProDOS blocks
- disk free space is back to `0 bytes`

Current resident / overlay occupancy from `make memory-usage`:

### Resident

| Area | Used | Free |
|------|-----:|-----:|
| `LOWCODE` | 5074 | 965 |
| `RESIDENT_MAIN_SAFE` | 16331 | 2101 |
| `LANGUAGE_CARD` | 2593 | 479 |

### Overlays

| Overlay | UsedApprox | FreeApprox |
|--------|-----------:|-----------:|
| `ascr.bin` | 1659 | 389 |
| `bscr.bin` | 1992 | 56 |
| `cnsl.bin` | 1164 | 884 |
| `dscr.bin` | 1978 | 70 |
| `iscr.bin` | 844 | 1204 |
| `menu.bin` | 917 | 1131 |
| `pscr.bin` | 1391 | 657 |
| `sscr.bin` | 1521 | 527 |
| `texp.bin` | 832 | 1216 |
| `tscr.bin` | 1938 | 110 |
| `txac.bin` | 1919 | 129 |

Important consequences:

- shrinking an overlay by 100 bytes does not reclaim floppy space by itself
- floppy space is reclaimed only if:
  - the main binary drops enough to free one or more ProDOS blocks, or
  - two overlays are merged so one 2 KB overlay file disappears
- the current regression from the last document snapshot is explained by:
  - `IIMPERIALISM` growing back from `64` to `66` blocks
  - adding the new `CNSL` overlay file, which costs another `5` blocks

## Fresh Conclusions

The best current options are no longer just "trim bytes somewhere."
They fall into two separate buckets:

### A. Reclaim a whole overlay file

This is still the cleanest immediate disk-space win.

One successful merge removes one `5`-block overlay file from disk. That is
`2560` bytes of real floppy space.

### B. Move resident code into an existing overlay file

This matters when the resident main binary is sitting on the wrong side of a
ProDOS block boundary, which it currently is.

If a resident screen can be moved into an overlay file that already exists and
already has room, that can reduce `IIMPERIALISM` without adding another file to
the floppy.

This is different from creating a brand-new overlay:

- moving code into a new overlay can reduce the main binary but still lose on
  disk because the new file also costs `5` blocks
- moving code into an existing overlay can reduce the main binary and avoid any
  new disk-file cost

## Best Overlay Merge Candidates

Current viable pairings by raw overlay occupancy:

| Candidate | Combined UsedApprox | Headroom |
|-----------|--------------------:|---------:|
| `ISCR + TEXP` | 1676 | 372 |
| `MENU + TEXP` | 1749 | 299 |
| `ISCR + MENU` | 1761 | 287 |
| `CNSL + TEXP` | 1996 | 52 |
| `CNSL + ISCR` | 2008 | 40 |

All other obvious pairs are over 2 KB and are not currently realistic.

### 1. Best pure disk-space merge: `ISCR + TEXP`

Current sizes:

- `iscr.bin` = `844`
- `texp.bin` = `832`
- combined = `1676`

Why this remains the best merge:

- it fits comfortably, with `372` bytes left for a dispatcher and minor growth
- both overlays already use the same resident warehouse/header helpers
- both already use `INDUSTRY_PORTRAIT`
- it deletes one overlay file from disk immediately

Why it is still slightly awkward:

- it is not a natural gameplay pair
- selection is not purely by overlay ID under the current ABI, so the merged
  file needs a dispatcher at `$8800`
- dispatch will likely be based on `state.current_screen`

Current ranking:

- best immediate disk-space win
- lowest-risk merge that still deletes a file

### 2. Best backup merge: `MENU + TEXP`

Current sizes:

- `menu.bin` = `917`
- `texp.bin` = `832`
- combined = `1749`

Why it is viable:

- still plenty of room for a dispatcher
- also deletes one overlay file from disk

Why I rank it below `ISCR + TEXP`:

- `MENU` is a transient `ESC` flow, not selected directly from
  `state.current_screen`
- that means the dispatcher has to key off something other than the normal
  screen value, or `run_overlay()` has to map multiple overlay IDs to one file
- it is workable, but not as straightforward

### 3. Pairings involving `CNSL`

Technically:

- `CNSL + TEXP` fits by `52` bytes
- `CNSL + ISCR` fits by `40` bytes

I do not recommend either as the first move.

Reason:

- the headroom is too small for comfort
- minor code growth, stub growth, or linker layout shifts could break the fit
- they are fine only if you are prepared to do a shrink pass first

## Moving Screens Out Of Resident Code

This is the new part that matters most for the current build.

The right question is not "can any resident screen become an overlay?"
The right question is:

- can it move into an already-existing overlay with spare space
- without adding a new disk file
- and with minimal resident glue left behind

### Ledger screen: yes, and it is now a strong candidate

Status: implemented

The ledger flow now lives inside `src/ovl_industry.c` and is reached by routing
`SCREEN_LEDGER` through `OVL_INDUSTRY`.

Previous resident map contribution from `ledger.o`:

- `CODE` = `0x020E` = `526` bytes
- `RODATA` = `0x00A6` = `166` bytes
- total direct resident contribution = about `692` bytes

Why this is attractive:

- the ledger is only entered from `src/ovl_industry.c`
- after closing, it already returns to `SCREEN_INDUSTRY`
- it uses the same `INDUSTRY_PORTRAIT` theme as the industry / transport /
  production family
- `iscr.bin` currently has `1204` bytes free, so the ledger fits comfortably

This made `ISCR` the natural home for the ledger flow.

Implemented shape:

1. move ledger rendering into the industry overlay binary
2. branch inside `render_industry_screen()` when `state.current_screen == SCREEN_LEDGER`
3. keep `SCREEN_LEDGER` as a screen ID
4. have resident `main()` send `SCREEN_LEDGER` through `run_overlay(OVL_INDUSTRY)`
5. export `print_signed_int_right_aligned_currency()` through `JMPTAB` so the
   overlay can preserve the existing signed-money formatting

Why I like this:

- it does not add a new overlay file
- it directly shrinks `IIMPERIALISM`
- it matches existing gameplay flow better than most other moves

Actual result:

- `IIMPERIALISM` dropped from `66` blocks to `65` blocks
- disk free space increased from `0 bytes` to `512 bytes`
- `iscr.bin` grew to `1824` bytes used, leaving `224` bytes free

New caveat after implementation:

- if you do this, `ISCR + TEXP` is no longer available as the first merge
  candidate, because `844 + 692 + 832 = 2368`, which is too large

So this creates a branch in the plan:

- either merge `ISCR + TEXP` first for the immediate `5`-block win
- or move ledger into `ISCR` first to claw back resident blocks

### Ledger screen in `MENU`: technically possible, but not the right fit

`menu.bin` has `1131` bytes free, so the ledger would also fit there.

I do not recommend it first.

Reason:

- the menu is not where ledger belongs semantically
- the ledger is only reached from industry
- using `MENU` would preserve the `ISCR + TEXP` merge option, but it makes the
  code organization less coherent

If the only goal were "keep `ISCR` small so it can merge with `TEXP` later,"
this is a possible compromise. It is not the cleanest architecture.

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
- move the ledger first if the goal is resident shrink
- revisit the main screen only after splash and startup responsibilities are
  disentangled

## Splash Screen Reassessment

The earlier document was correct that the splash is still one of the largest
resident-only feature costs, because `SHIP_SPLASH` is only used once at boot.

That remains true, but the disk-space context has changed:

- adding a new splash overlay right now would cost another `5` blocks on a disk
  that already has `0 bytes free`
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
current `0 bytes free` pressure.

### Replace the 192-entry `HGR_ROWS[]` table

Still plausible resident win:

- about `300+` bytes feels realistic

But today it is behind:

- one-file overlay merge
- ledger-to-overlay move
- splash relocation that does not add a file

### Revisit resident-only strings

Still worth asking:

- are any resident strings only serving one overlay that already has room

But this is now a secondary tuning pass, not the primary strategy.

### Bold font derivation / picture compression

Still potentially useful, but still higher risk than the options above.

## Recommended Order

There are now two sensible paths, depending on whether the first priority is
"free the most blocks quickly" or "shrink resident first."

### Path A: fastest disk recovery

1. merge `ISCR + TEXP`
2. re-run `make memory-usage` and disk catalog
3. if more space is still needed, consider moving ledger into `MENU` or another
   roomy existing overlay instead of growing `ISCR`
4. revisit splash relocation without adding a new disk file

Why this path ranks first:

- it frees `5` blocks immediately
- it is the clearest way off a completely full floppy

### Path B: resident-first cleanup

1. move ledger into `ISCR`
2. verify whether `IIMPERIALISM` drops from `66` to `65` or lower blocks
3. if more space is still needed, merge `MENU + TEXP`
4. then revisit splash relocation without adding a new disk file

Why this path is attractive:

- it improves the architecture around the industry flow
- it removes an isolated resident screen that does not need to stay resident
- it may recover disk blocks from the main binary without adding any new file

Why I still rank it second:

- the disk is at literal `0 bytes free`
- a guaranteed `5`-block merge is more valuable than a likely `1`-block resident drop

## Final Recommendation

If I had to pick the best next move today, it would be:

1. merge `ISCR + TEXP` to reclaim one full overlay file
2. after that, move the ledger into an existing overlay file if the main binary
   still needs to come down

If the goal is specifically to move resident screens out of the main binary,
the ledger is the right screen to move first, not the main screen.

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
- merged overlays still dispatch to the correct screen
- save/load still works if `MENU` is touched
