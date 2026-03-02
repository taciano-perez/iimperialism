# Memory Management for IImperialism

## Apple II Memory Map

```
$0000-$00FF  Zero Page          (cc65 runtime; $9A-$9E reserved for trampoline params)
$0100-$0117  RAMRD trampoline   (installed by `install_trampoline()` at startup)
$0118-$01FF  6502 hardware stack
$0200-$03FF  System / ProDOS vectors
$0400-$07FF  Text screen page 1
$0803-$080E  STARTUP            (cc65 crt0)
$080F-$0823  JMPTAB             (resident jump table used by overlays)
$0824-$1FFF  LOWCODE            (resident UI + core logic)
$2000-$3FFF  HGR page 1         (graphics memory; no code here)
$4000-...    CODE/RODATA/DATA   (main resident code + data)
...          BSS/ONCE/heap
$8800-$8FFF  OVERLAY_SLOT       (2KB execution window in main RAM)
$8E00-$95FF  C stack            (2KB, downward from HIMEM=$9600)
$9600-$BEFF  ProDOS system area
$BF00-$BFFF  ProDOS MLI

AUX RAM (second 64KB bank):
$8800-$8FFF  OVERLAY_AUX_SLOT   (single 2KB cached overlay copy)
$9000-$BEFF  Available AUX RAM  (currently unused by loader)
```

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
- `ASCR` admiralty screen
- `ATRD` admiralty build-trader flow
- `AWRS` admiralty build-warship flow

Runtime flow (`run_overlay(id)`):

1. Map overlay ID to ProDOS filename.
2. Load exactly 2048 bytes from disk into main RAM `OVERLAY_SLOT` (`$8800`).
3. Copy main RAM `$8800-$8FFF` to AUX RAM at the same address (`main_to_aux`).
4. Set ZP trampoline parameters (`$9A-$9E`) for AUX -> MAIN copy.
5. Call trampoline at `$0100` to copy AUX -> MAIN with RAMRD enabled.
6. Execute overlay entry at `$8800`, passing `&state`.

Why trampoline code is at `$0100`:

- With RAMRD on, instruction fetches in `$0200-$BFFF` come from AUX.
- Stack page `$0100-$01FF` is not switched by RAMRD, so trampoline code remains
  executable from main memory during the copy.

## Zero Page Trampoline Parameters

`overlay.h` reserves `$9A-$9E`:

- `$9A/$9B` `tramp_src` (AUX source address)
- `$9C/$9D` `tramp_dst` (MAIN destination address)
- `$9E` `tramp_pages` (page count; 8 for 2KB)

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
```

Rule: never change existing entry addresses. Append only.

## Adding a New Overlay

Example: diplomacy overlay as ID `6`, file `DSCR`.

1. Create `src/ovl_diplomacy.c` with entry `void render_diplomacy_screen(GameState *s)`.
2. Add to `include/overlay.h`:

```c
#define OVL_DIPLOMACY 6
#define OVL_FILE_DIPLOMACY "DSCR"
```

3. Extend `overlay_filename()` in `src/overlay.c`:

```c
case OVL_DIPLOMACY: return OVL_FILE_DIPLOMACY;
```

4. Add object and binary rules in `Makefile`:

```makefile
$(BUILD_DIR)/ovl_diplomacy.o: $(SRC_DIR)/ovl_diplomacy.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $<
	$(MV_CMD) $(SRC_DIR)/ovl_diplomacy.o $@

$(BUILD_DIR)/dscr.bin: $(BUILD_DIR)/ovl_diplomacy.o | $(BUILD_DIR)
	$(OVL_CC) $(OVL_LDFLAGS) -o $(BUILD_DIR)/dscr.bin $(BUILD_DIR)/ovl_diplomacy.o
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
`include/game.h`.

## Expansion Areas

| Area | Address | Capacity | Notes |
|------|---------|----------|-------|
| LOWCODE | `$0824-$1FFF` | ~3.9KB free (approx) | Most effective for reducing resident pressure |
| AUX RAM | `$9000-$BEFF` | ~12KB | Available for future cache/expansion |
| Language Card | `$D400-$DFFF` | 3KB | Separate RAM bank |

## cc65 Configuration Reference

Key symbols in `config/apple2-hgr.cfg`:

| Symbol | Value | Purpose |
|--------|-------|---------|
| `__HIMEM__` | `$9600` | Top of C stack region |
| `__STACKSIZE__` | `$0800` | 2KB C stack |
| `__LCADDR__` | `$D400` | Language card start |
| `__LCSIZE__` | `$0C00` | Language card size |
