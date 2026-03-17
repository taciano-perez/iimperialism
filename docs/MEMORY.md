# Memory Management for IImperialism

## Apple II Memory Map

```
$0000-$00FF  Zero Page          (cc65 runtime)
$0100-$01FF  6502 hardware stack
$0200-$03FF  System / ProDOS vectors
$0400-$07FF  Text screen page 1
$0803-$080E  STARTUP            (cc65 crt0)
$080F-$0841  JMPTAB             (resident jump table used by overlays)
$0824-$1FFF  LOWCODE            (resident UI + core logic)
$2000-$3FFF  HGR page 1         (graphics memory; no code here)
$4000-...    CODE/RODATA/DATA   (main resident code + data)
...          BSS/ONCE/heap
$8800-$8FFF  OVERLAY_SLOT       (2KB execution window in main RAM)
$8E00-$95FF  C stack            (2KB, downward from HIMEM=$9600)
$9600-$BEFF  ProDOS system area
$BF00-$BFFF  ProDOS MLI
```

Current resident note:

- `print()` uses the aligned HGR text blitter in `asm/text_hgr.s`, which lives in
  `LOWCODE` and trades a small amount of resident code and lookup-table data for
  much faster text rendering

## Why the Main Binary Is Large

The main binary includes a zero-filled gap between LOWCODE and CODE because:

- HGR requires code to start at `$4000`
- `$2000-$3FFF` is graphics memory
- LOWCODE only uses part of `$0824-$1FFF`

As string literals grow, `RODATA/DATA/INIT` move upward. When image size and memory
pressure cross a threshold, BRUN can fail silently back to ProDOS.

## Overlay Architecture

Overlays are standalone 2KB binaries stored on disk and loaded on demand.

Current overlay files:

- `ISCR` industry screen
- `PSCR` production screen
- `TSCR` transport screen
- `ASCR` admiralty screen and build flow
- `DSCR` diplomacy screen
- `TEXP` diplomacy trade expedition market screen
- `TXAC` diplomacy trade expedition action flow
- `BSCR` battle screen
- `SSCR` science screen

To inspect current resident segment usage plus overlay occupancy from build
artifacts, run:

```bash
make memory-usage
```

The memory-usage report distinguishes between:

- `RESIDENT_MAIN_SAFE` for resident main-binary content that must stay below
  `$8800`
- `OVERLAY_SLOT` for the overlay load/execution window at `$8800-$8FFF`

This matters because resident-code growth into `$8800-$8FFF` can break overlay
loading even if the older, broader high-memory totals still appeared to fit.

Runtime flow (`run_overlay(id)`):

1. Map overlay ID to ProDOS filename.
2. Call the resident ProDOS loader helper in `asm/prodos_overlay_load.s`.
3. Use ProDOS MLI `OPEN` / `READ` / `CLOSE` to load exactly 2048 bytes into
   main RAM `OVERLAY_SLOT` (`$8800`).
4. Execute overlay entry at `$8800` (no arguments; overlays access `state`
   directly via the `_state` symbol exported in the generated overlay linker
   config).

The runtime overlay path intentionally avoids `fopen()` / `fread()`. Direct
MLI calls are more robust under the game's current resident memory pressure
than the Apple II `stdio` path.

For overlays with helper functions, use an explicit assembly entry stub so the
intended entry point stays at `$8800` even if function ordering changes during
compilation. Current examples:

- `asm/ovl_diplomacy_entry.s` -> `render_diplomacy_screen()`
- `asm/ovl_trade_expedition_entry.s` -> `render_trade_market()`
- `asm/ovl_trade_expedition_action_entry.s` -> `handle_screen_trade_expedition()`
- `asm/ovl_science_entry.s` -> `render_science_screen()`

## Resident Jump Table (`$080F`)

Overlay binaries call resident functions through fixed JMP entries:

```
$080F  JMP _clear_screen
$0812  JMP _clear_input_area
$0815  JMP _print
$0818  JMP _print_int_right_aligned
$081B  JMP _draw_picture_at
$081E  JMP _box
$0821  JMP _render_warehouse_box
$0824  JMP _cgetc
$0827  JMP _scan_uint
$082A  JMP _print_int
$082D  JMP _get_resource_name
$0830  JMP _get_relation_name
$0833  JMP _set_selected_trade_nation
$0836  JMP _get_selected_trade_nation
$0839  JMP _clear_area
$083C  JMP _paint_area
$083F  JMP _rand_range
```

Rule: keep `asm/jmptab.s` and `config/apple2-ovl.cfg` in sync. Rebuild overlays
after any jump-table change so they relink against the new addresses.

## `_state` Address in Overlay Config

Overlays access `GameState state` directly via `_state`.

`config/apple2-ovl.cfg` is now a template. During the build, `Makefile` generates
`build/apple2-ovl.cfg` by reading the current `_state` address from
`build/iimperialism.map` and rewriting the `_state` symbol before linking overlays.

This address is still determined by the combined size of resident `CODE + RODATA +
DATA + INIT`, so any resident-code size change can move it.

Practical rules:

- do a full rebuild after resident-code changes
- use `make memory-usage` to confirm the resident layout
- if overlays show garbage values, compare `_state` in `build/iimperialism.map` and
  `build/apple2-ovl.cfg`

A stale `_state` address causes overlays to read `GameState` fields from the wrong
offset, producing silent runtime corruption even when the overlay code itself is correct.

## Adding a New Overlay

Example: diplomacy overlay as ID `6`, file `DSCR`.

1. Create `src/ovl_diplomacy.c` with entry `void render_diplomacy_screen(void)`.
   Access game state via the global `state`; no parameter needed.
2. Add to `include/overlay.h`:

```c
#define OVL_DIPLOMACY 6
#define OVL_FILE_DIPLOMACY "DSCR"
```

3. Extend `overlay_filename()` in `src/overlay.c`:

```c
case OVL_DIPLOMACY: return OVL_FILE_DIPLOMACY;
```

4. Add object and binary rules in `Makefile`.

The resident overlay loader does not need changes unless the new overlay file
name requires a different filename mapping.

If the overlay contains multiple functions, add an assembly entry stub and link it
first so the correct entry symbol lands at `$8800`:

```asm
    .export _ovl_example_entry
    .import _render_example_screen

    .segment "CODE"

_ovl_example_entry:
    jmp _render_example_screen
```

Then add the stub object and binary rules in `Makefile`:

```makefile
$(BUILD_DIR)/ovl_example_entry.o: $(ASM_DIR)/ovl_example_entry.s | $(BUILD_DIR)
	ca65 $(ASM_DIR)/ovl_example_entry.s -o $(BUILD_DIR)/ovl_example_entry.o

$(BUILD_DIR)/escr.bin: $(BUILD_DIR)/ovl_example_entry.o $(BUILD_DIR)/ovl_example.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/escr.bin $(BUILD_DIR)/ovl_example_entry.o $(BUILD_DIR)/ovl_example.o
```

5. Add `$(BUILD_DIR)/dscr.bin` to `overlays` target.
6. Add disk entry to `disk` target:

```makefile
-$(AC) -d $(DISK) DSCR
$(AC) -p $(DISK) DSCR BIN 0x8800 < $(BUILD_DIR)/dscr.bin
```

7. Trigger it from resident code via `run_overlay(OVL_DIPLOMACY)`.

If the overlay needs a resident function not in JMPTAB, append a new JMP entry in
`asm/jmptab.s`, export it from `config/apple2-ovl.cfg`, and declare it in
`include/game.h`. Current examples include `clear_area(int x, int y, int width, int height)`
at `$0839`.

## Expansion Areas

| Area | Address | Capacity | Notes |
|------|---------|----------|-------|
| LOWCODE | `$0824-$1FFF` | ~3.9KB free (approx) | Most effective for reducing resident pressure |
| Language Card | `$D400-$DFFF` | 3KB | Separate RAM bank |

Use `make memory-usage` to get the current used/free breakdown for resident
memory windows and overlay binaries from the latest build.

## cc65 Configuration Reference

Key symbols in `config/apple2-hgr.cfg`:

| Symbol | Value | Purpose |
|--------|-------|---------|
| `__HIMEM__` | `$9600` | Top of C stack region |
| `__STACKSIZE__` | `$0800` | 2KB C stack |
| `__LCADDR__` | `$D400` | Language card start |
| `__LCSIZE__` | `$0C00` | Language card size |
