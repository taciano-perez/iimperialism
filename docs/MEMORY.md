# Memory Management for IImperialism

## Apple II Memory Map

```
$0000-$00FF  Zero Page          (cc65 runtime: 26 bytes; $9A-$9E reserved for overlay trampoline)
$0100-$0117  RAMRD Trampoline   (18 bytes of 6502 code, installed at startup by install_trampoline)
$0118-$01FF  6502 Hardware Stack
$0200-$03FF  System / ProDOS page 3 vectors
$0400-$07FF  Text screen page 1
$0803-$080E  STARTUP            (12 bytes — cc65 crt0 init, vectors into LOWCODE)
$080F-$0823  JMPTAB             (21 bytes — 7 × JMP entries, fixed address, used by overlays)
$0824-$1FFF  LOWCODE            (~7.6KB available; ~3.7KB used by ui.c, logic.c, gamestate.c, overlay.c)
$2000-$3FFF  HGR screen page 1  (TGI — off limits for code)
$4000-...    CODE               (main.c — screen dispatch, input handlers, render_warehouse_box)
...          RODATA             (string literals)
...          DATA               (initialized globals)
...          INIT / ONCE / BSS
...          Heap               (grows upward from ~$8160)
$8800-$8FFF  OVERLAY_SLOT       (2KB execution window; overwritten on each run_overlay call)
$8E00-$95FF  C stack            (2KB, grows downward from HIMEM=$9600)
$9600-$BEFF  ProDOS system area
$BF00-$BFFF  ProDOS MLI

AUX RAM (second 64KB bank, accessed via RAMRD/RAMWRT soft switches):
$8800-$8FFF  OVERLAY_AUX_SLOT    (single 2KB overlay cache/execution source for trampoline copy)
$9000-$BEFF  Available AUX RAM   (currently unused by overlay loader; room for future caching/expansion)
```

## Why the Binary Is Large

The binary file contains everything from `$0803` to the end of `INIT` — including a
**~14KB gap of zeros** between `$0824` and `$3FFF`. This gap exists because:

- CODE must start at `$4000` (required by TGI for HGR page 1)
- The HGR screen at `$2000-$3FFF` cannot hold code (TGI writes pixels there)
- The space `$0824-$1FFF` is partially used by LOWCODE, with ~3.9KB still available

Every `print()` call adds a string literal to RODATA, which pushes DATA, INIT, and
ONCE upward, growing the binary end address toward the stack. Once it hits a critical
threshold, the program silently returns to the ProDOS BASIC prompt on BRUN.

## Current Allocation

LOWCODE (~3.7KB used of 7.6KB available):
- `ui.c`: ~1.9KB
- `logic.c`: ~1.1KB
- `gamestate.c`: ~0.6KB
- `overlay.c`: ~0.1KB
- cc65 lib (cout/rdkey/vtabz): ~0.1KB

```
Segment   Start    End      Size
-------   -----    ---      ----
STARTUP   $0803    $080E    12B
JMPTAB    $080F    $0823    21B
LOWCODE   $0824    ~$18xx   ~3.7KB  (~3.9KB spare)
CODE      $4000    ~$670E   ~9.8KB  (was ~12.5KB before overlays)
RODATA    ~$670E   ~$75BE   ~3.3KB
DATA      ~$75BE   ~$76DE   ~300B
INIT      ~$76DE   ~$76FC   ~30B
BSS/ONCE  ~$76FC   ~$7860   ~360B
Heap      ~$7860   $87FF    ~4KB free
```

Binary file size: ~31KB (including the ~14KB gap).

## Warning Signs

- Program silently returns to ProDOS BASIC prompt on BRUN
- Adding a single `print()` call breaks execution
- No compiler or linker error — the binary builds cleanly

These symptoms indicate the binary end address has grown past a critical threshold.
**Action:** move the next logical unit of code to LOWCODE or add a new overlay screen.

## Overlay Architecture

The overlay screen renderers (currently industry, production, transport, and
admiralty) are compiled as standalone 2KB binaries stored on the ProDOS disk
(`ISCR`, `PSCR`, `TSCR`, `ASCR`) and loaded on demand.

**Startup sequence (`init_overlays()` in overlay.c):**
1. `install_trampoline()` — copies 18-byte trampoline code into $0100-$0117
2. No overlays are preloaded at startup

**Screen switch (`run_overlay(id)` in overlay.c):**
1. Map overlay ID to a ProDOS filename (`ISCR`, `PSCR`, `TSCR`, `ASCR`)
2. `fopen/fread` the overlay file into OVERLAY_SLOT (MAIN `$8800`)
3. `main_to_aux()` (via RAMWRT) copies 8 pages from MAIN `$8800` to AUX `$8800`
4. Set ZP trampoline params (`$9A-$9E`): AUX source `$8800`, MAIN dest `$8800`, 8 pages
5. Call trampoline at `$0100` (enables RAMRD, copies 8 pages AUX→MAIN, disables RAMRD)
6. Call `((void(*)(GameState*))0x8800)(&state)`

This preserves the existing trampoline execution path while scaling the number of
overlays with disk space instead of fixed AUX slots.

**Why the trampoline lives in the hardware stack page ($0100):**
When RAMRD is on, instruction fetches from `$0200-$BFFF` come from AUX RAM. The
hardware stack page (`$0100-$01FF`) is controlled by ALTZP (not RAMRD), so code
there always executes from MAIN regardless of RAMRD state.

**Zero-page layout (`$9A-$9E`, outside cc65's ZP range of `$80-$99`):**
```
$9A/$9B  tramp_src   — AUX source address = $8800 (set by run_overlay)
$9C/$9D  tramp_dst   — MAIN destination address = $8800 (set by run_overlay)
$9E      tramp_pages — page count = 8 (set by run_overlay)
```

## Jump Table (`$080F`, 7 entries, 21 bytes)

The JMPTAB segment occupies `$080F-$0823`. Overlay binaries resolve their external
calls to these fixed addresses via the `SYMBOLS` section in `apple2-ovl.cfg`:

```
$080F  JMP _clear_screen
$0812  JMP _clear_input_area
$0815  JMP _print
$0818  JMP _print_int_right_aligned
$081B  JMP _draw_picture_at
$081E  JMP _box
$0821  JMP _render_warehouse_box
```

**Rule:** never change existing entry addresses. New entries are always appended
(at `$0824`, `$0827`, ...), which shifts LOWCODE up by 3 bytes per entry.

## Adding a New Screen as an Overlay

Example: adding a diplomacy screen with overlay ID 4, disk file DSCR.

### 1. Create `ovl_diplomacy.c`

```c
#include "game.h"
#define state (*s)

void render_diplomacy_screen(GameState *s) {
    clear_screen();
    /* ... box(), print(), print_int_right_aligned() calls ... */
}
```

All calls to `clear_screen`, `print`, `box`, etc. resolve automatically
via the jump table. No `#include "ui.h"` needed.

### 2. Add constants to `overlay.h`

```c
#define OVL_DIPLOMACY        4
#define OVL_FILE_DIPLOMACY   "DSCR"
```

### 3. Add filename mapping to `overlay.c` → `overlay_filename()`

```c
case OVL_DIPLOMACY: return OVL_FILE_DIPLOMACY;
```

### 4. Add build rules to `Makefile`

Add to `overlays` target:
```makefile
overlays: iscr.bin pscr.bin tscr.bin ascr.bin dscr.bin
```

Add compile and link rules:
```makefile
ovl_diplomacy.o: ovl_diplomacy.c game.h
	$(CC) $(CFLAGS) -c ovl_diplomacy.c -o ovl_diplomacy.o

dscr.bin: ovl_diplomacy.o
	$(OVL_CC) -o dscr.bin ovl_diplomacy.o
```

Add to `disk` target:
```makefile
$(AC) -d $(DISK) DSCR 2>/dev/null; $(AC) -p $(DISK) DSCR BIN 0x8800 < dscr.bin
```

Add `dscr.bin` and `ovl_diplomacy.o` to the `clean` target.

### 5. Call the overlay from `main.c`

```c
run_overlay(OVL_DIPLOMACY);
```

### 6. (Optional) Add resident functions to the jump table

If the new screen needs to call a function not already in JMPTAB
(e.g., `draw_map()`), it must be made resident and exposed via the jump table:

**`jmptab.s`** — append at the end:
```asm
    jmp _draw_map           ; $0824
```

**`apple2-ovl.cfg`** — append to SYMBOLS:
```
_draw_map: type = export, value = $0824;
```

**`game.h`** — add declaration:
```c
void draw_map(void);
```

The function itself lives in `main.c` (CODE) or `ui.c` (LOWCODE).
After adding one JMP entry, LOWCODE shifts from `$0824` to `$0827` —
verify with `iimperialism.map` after the next build.

## Available Expansion Areas

| Area | Address | Capacity | Notes |
|------|---------|----------|-------|
| LOWCODE | `$0824–$1FFF` | ~3.9KB free | Most effective — shrinks binary end address |
| AUX RAM | `$9000–$BEFF` | ~12KB | Free now; can be used for overlay cache or other expansion |
| Language Card | `$D400–$DFFF` | 3KB | Separate RAM bank, copied at startup |

## cc65 Configuration Reference

Key symbols in `apple2-hgr.cfg`:

| Symbol | Value | Purpose |
|--------|-------|---------|
| `__HIMEM__` | `$9600` | Upper boundary of C stack (keeps stack below OVERLAY_SLOT at $8800) |
| `__STACKSIZE__` | `$0800` | 2KB C stack ($8E00–$95FF) |
| `__LCADDR__` | `$D400` | Language Card start address |
| `__LCSIZE__` | `$0C00` | Language Card size (3KB) |
