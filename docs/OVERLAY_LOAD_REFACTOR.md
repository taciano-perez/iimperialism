# Overlay Load Refactor Notes

## Problem Summary

Overlay loading started failing at runtime after enlarging `GameState` with
price arrays for foreign trade.

Symptoms:

- The game initially failed on `DSCR` at startup.
- After changing startup to industry, it also failed on `ISCR`.
- The on-screen diagnostic was improved in `src/overlay.c` and reported:

```text
fread bytes: 0
```

This means:

- `fopen()` succeeded, so the overlay file was found on disk.
- `fread()` immediately returned EOF / no data.
- The failure was not specific to one overlay file.

## What Was Ruled Out

### Not a missing overlay file

`make disk` and `ac -l assets/iimperialism.dsk` showed all overlays present:

- `ISCR`
- `PSCR`
- `TSCR`
- `ASCR`
- `ATRD`
- `AWRS`
- `DSCR`
- `TEXP`

All corresponding `build/*.bin` files were exactly 2048 bytes.

### Not LOWCODE overflow

`make memory-usage` showed LOWCODE still had free space. At the time of
investigation:

- `LOWCODE`: 4477 used / 6093 total
- free: 1616 bytes

So this was not a direct LOWCODE exhaustion problem.

### Not overlay entry-point ordering

The diplomacy and trade-expedition overlays use explicit assembly entry stubs,
and the failure occurred before execution reached overlay code.

## Key Finding

The regression was caused by enlarging `GameState`, not by `rand()` itself.

Specifically:

- adding `export_prices[]` and `import_prices[]` to `ForeignNation`
- initializing those values during `init_game()`

made the overlay loader fail.

When the price fields were temporarily removed from `GameState`, overlay loading
started working again.

## Likely Root Cause

The current overlay loader in `src/overlay.c` uses:

- `fopen(filename, "rb")`
- `fread((void*)OVERLAY_SLOT, 1, OVERLAY_SIZE, f)`

This `stdio`-based path appears to be too fragile under current Apple II /
cc65 memory pressure.

Why this is the likely issue:

- `fopen()` succeeds but `fread()` returns 0
- removing the added game-state fields restores overlay loading
- cc65 Apple II `stdio` uses ProDOS I/O buffers and heap-backed runtime support
- the project is already relatively tight on resident memory

So the likely explanation is that the current `stdio` overlay-load path is
sensitive to memory/layout pressure, even though the overlay files themselves
are valid and present.

## Recommended Refactor

Replace overlay loading in `src/overlay.c` with a lower-level ProDOS read path
instead of `stdio`.

Preferred approach:

- use ProDOS MLI `OPEN` / `READ` / `CLOSE` directly
- read exactly 2048 bytes into `OVERLAY_SLOT`

Reason:

- the loader in `asm/loader/loader.s` already uses this style successfully
- it should be more robust than the current `fopen` / `fread` approach
- it should decouple overlay loading from fragile `stdio`/heap behavior

## Suggested Implementation Direction

1. Keep the existing overlay ID -> filename mapping in `src/overlay.c`.
2. Replace `load_overlay_file()` with a ProDOS/MLI-backed implementation.
3. Reuse patterns from `asm/loader/loader.s`:
   - pathname buffer
   - `OPEN`
   - `READ`
   - `CLOSE`
4. Preserve the current post-load behavior:
   - copy MAIN -> AUX
   - trampoline AUX -> MAIN
   - jump to overlay entry at `$8800`

## Temporary Debugging Outcome

For the debugging session that produced this note:

- prices were removed from `GameState`
- startup was temporarily set to `SCREEN_INDUSTRY`
- overlay loading worked again after removing the price fields

That temporary rollback confirmed the diagnosis, but it is not the desired
long-term architecture.
