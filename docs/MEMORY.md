# Memory Management for IImperialism

## Apple II Memory Map

```
$0000-$00FF  Zero Page          (cc65 runtime)
$0100-$01FF  6502 hardware stack
$0200-$03FF  System / ProDOS vectors
$0400-$07FF  Text screen page 1
$0803-$080E  STARTUP            (cc65 crt0)
$080F-$0871  JMPTAB             (resident jump table used by overlays)
$0872-$1FFF  LOWCODE            (resident main-RAM helpers + core logic)
$2000-$3FFF  HGR page 1         (graphics memory; no code here)
$4000-...    CODE/RODATA/DATA   (main resident code + data)
...          BSS/ONCE/heap
$8800-$8FFF  OVERLAY_SLOT       (2KB execution window in main RAM)
$8E00-$95FF  C stack            (2KB, downward from HIMEM=$9600)
$9600-$BBFF  C stack / high-memory workspace
$BC00-$BFFF  Resident ProRWTS runtime
$D400-$DD79  LC                 (`src/ui.c` UI code in Language Card RAM)
```

Current resident note:

- `print()`, `print_bold()`, and `print_inverted()` all render through aligned HGR
  text blitters in `asm/text_hgr.s`
- those blitters live in main-RAM `LOWCODE`, while the higher-level `src/ui.c`
  wrappers and input helpers now live in the `LC` segment in Language Card RAM
- this split preserves scarce main-RAM resident space while keeping overlay-callable
  entry points stable through JMPTAB

## Why the Main Binary Is Large

The main binary includes a zero-filled gap between LOWCODE and CODE because:

- HGR requires code to start at `$4000`
- `$2000-$3FFF` is graphics memory
- LOWCODE only uses part of the pre-HGR main-RAM region

Separately, the linker can place selected code into the Apple II Language Card:

- `config/apple2-hgr.cfg` defines `LC` at `$D400-$DFFF`
- `src/ui.c` currently uses `#pragma code-name (push, "LC")`
- the current build map places `ui.o` `LC` code at `$D400-$DD79`

As string literals grow, `RODATA/DATA/INIT` move upward. When image size and memory
pressure cross a threshold, the shipping boot/runtime path can lose the headroom
it needs for overlays, stack, and resident ProRWTS.

## Overlay Architecture

Overlays are standalone 2KB binaries stored on disk and loaded on demand.

Current overlay files:

- `ISCR` industry screen
- `PSCR` production screen
- `TSCR` transport screen
- `ASCR` admiralty screen and build flow
- `DSCR` diplomacy screen
- `TXAC` diplomacy trade expedition action flow
- `BSCR` battle screen
- `SSCR` science screen
- `MENU` game menu screen
- `CNSL` Council of Nations and final victory report

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

1. Map overlay ID to on-disk filename.
2. Call the resident disk loader helper for the active backend.
3. Use resident ProRWTS file reads to load exactly 2048 bytes into main RAM
   `OVERLAY_SLOT` (`$8800`).
4. Execute overlay entry at `$8800` (no arguments; overlays access `state`
   directly via the `_state` symbol exported in the generated overlay linker
   config).

The runtime overlay path intentionally avoids `fopen()` / `fread()`. The
resident ProRWTS path is more robust under the game's current resident memory
pressure than the Apple II `stdio` path.

Game-state persistence now lives entirely in the game menu overlay.
`save_game()` / `load_game()` and the disk helper use resident ProRWTS access
to `GAME.DATA` instead of linking the heavier `stdio` path into resident code.
Current disk helpers:

- `asm/disk_overlay_load.s`
- `asm/disk_gamestate_io.s`

The boot path leaves a write-capable ProRWTS runtime resident at `$BC00`, and
both overlay loading and the fixed-size `GAME.DATA` save container use that
runtime directly.

Current save-container details:

- `GameState` payload size is `186` bytes.
- Each slot record is `189` bytes: a 3-byte slot header plus the payload.
- The five-slot file is `949` bytes total: a 4-byte container header plus five
  189-byte slot records. It still fits in two 512-byte ProDOS blocks.
- Container version is `3`; per-slot save header version is `4`.

For overlays with helper functions, use an explicit assembly entry stub so the
intended entry point stays at `$8800` even if function ordering changes during
compilation. Current examples:

- `asm/ovl_diplomacy_entry.s` -> `render_diplomacy_screen()`
- `asm/ovl_industry_entry.s` -> `render_industry_screen()`
- `asm/ovl_production_entry.s` -> `render_production_screen()`
- `asm/ovl_transport_entry.s` -> `render_transport_screen()`
- `asm/ovl_trade_expedition_action_entry.s` -> `handle_screen_trade_expedition()`
- `asm/ovl_science_entry.s` -> `render_science_screen()`
- `asm/ovl_game_menu_entry.s` -> `render_game_menu_screen()`
- `asm/ovl_council_nations_entry.s` -> `render_council_nations_screen()`

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
$0842  JMP _start_new_game
$0845  JMP _print_int_right_aligned_currency
$0848  JMP _render_turn_funds_header
$084B  JMP _print_bold
$084E  JMP _wait_three_seconds_or_keypress
$0851  JMP _play_sound
$0854  JMP _play_sound_alert
$0857  JMP _cgetc_at
$085A  JMP _run_overlay
$085D  JMP _production_orders
$0860  JMP _print_inverted
$0863  JMP _get_diplomacy_string
$0866  JMP _print_signed_int_right_aligned_currency
$0869  JMP _build_final_score_line
$086C  JMP _get_final_rank_index
$086F  JMP _get_final_victory_string
```

Rule: keep `asm/jmptab.s` and `config/apple2-ovl.cfg` in sync. Rebuild overlays
after any jump-table change so they relink against the new addresses.

Even though much of `src/ui.c` now lives in `LC`, overlays still call resident UI
helpers through these fixed JMPTAB addresses. The jump-table entries decouple the
overlay ABI from the actual placement of the target code.

Current note:

- `src/ovl_industry.c` now handles its own input loop entirely inside the overlay
- `src/ovl_transport.c` now handles both its screen loop and transport actions fully
  inside the overlay
- `src/ovl_production.c` handles its screen loop and training flow inside the overlay,
  while `src/production.c` still provides resident `production_orders()` through JMPTAB
- `src/ovl_industry.c` now also owns the ledger sub-screen, while resident code
  routes `SCREEN_LEDGER` back through `OVL_INDUSTRY`
- `src/trade_expedition.c` owns the resident trade expedition market renderer;
  the separate `TEXP` overlay was removed, and `SCREEN_TRADE_EXPEDITION` now
  calls `render_trade_market()` before loading `TXAC`

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

3. Extend the active disk backend's overlay-name table.

Current disk backend:

```asm
OVL_NAME_DIPLOMACY:
    .byte .strlen("DSCR")
    .byte "DSCR"
```

4. Add object and binary rules in `Makefile`.

The resident overlay loader entry point does not need changes unless the active
backend requires different overlay metadata.

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
at `$0839`, `print_bold(unsigned char x, unsigned char y, const char* text)` at `$084B`,
`print_inverted(unsigned char x, unsigned char y, const char* text)` at `$0860`, and
`get_diplomacy_string(unsigned char index)` at `$0863`,
`print_signed_int_right_aligned_currency(unsigned char x, unsigned char y, int value)`
at `$0866`, and the Council final-report helpers
`build_final_score_line(char* buffer)`, `get_final_rank_index()`, and
`get_final_victory_string(unsigned char index)` at `$0869`, `$086C`, and `$086F`.

## Council Victory Screen Footprint

The victory report intentionally stays inside the existing `CNSL` overlay. Adding
a separate victory overlay would cost another fixed 2 KB overlay file on disk and
is not compatible with the current floppy-space pressure.

The implementation splits responsibilities to keep `cnsl.bin` under 2 KB:

- rendering and screen orchestration remain in `src/ovl_council_nations.c`
- score calculation, rank selection, and score-line formatting live in resident
  `src/logic.c`
- final-report strings live in resident `src/strings.c` and are exposed through
  `get_final_victory_string()`
- all score tuning constants live in `include/game.h`
- final score inputs are limited to diplomacy, speed, and treasury; sea power,
  merchant capacity, and science are displayed report values only

This is a deliberate size tradeoff. Moving score formatting into the overlay was
larger than using the resident `build_final_score_line()` helper, so the helper is
kept resident and exposed through `JMPTAB`.

## Expansion Areas

| Area | Address | Capacity | Notes |
|------|---------|----------|-------|
| LOWCODE | `$0872-$1FFF` currently used | main RAM below HGR | Compact resident helpers, blitters, overlay loader, core logic |
| Language Card | `$D400-$DFFF` | 3KB | Separate RAM bank; currently hosts `src/ui.c` code (`$D400-$DD79` used) |

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
