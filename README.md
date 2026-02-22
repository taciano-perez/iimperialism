# IImperialism - Apple II Strategy Game (cc65)

An early-stage strategy game for the Apple II, written in C using the **cc65** compiler
and its Tiny Graphics Interface (TGI) for Hi-Res Graphics (HGR) mode.

## Overview

The game uses a custom memory layout to fit within the Apple II's tight constraints:
- Screen renderers are compiled as standalone 2KB **AUX RAM overlays** (ISCR, PSCR, TSCR)
  loaded at startup and paged in on demand, freeing CODE space in MAIN RAM.
- All UI primitives and game logic live in **LOWCODE** (`$0824-$1FFF`), below the HGR
  screen, keeping the critical CODE segment small.

See `docs/MEMORY.md` for the full memory map and overlay architecture.

## File Structure

| File | Purpose |
|------|---------|
| `src/main.c` | Screen dispatch, input handlers, `render_warehouse_box` |
| `src/ui.c` | All UI primitives: `print`, `box`, `clear_screen`, etc. (LOWCODE) |
| `src/logic.c` | `init_game()`, `next_turn()` (LOWCODE) |
| `src/gamestate.c` | Save / load game state via ProDOS file I/O (LOWCODE) |
| `src/overlay.c` | `init_overlays()`, `run_overlay()` - overlay loading and dispatch (LOWCODE) |
| `src/ovl_industry.c` | `render_industry_screen` - compiled to `iscr.bin` |
| `src/ovl_production.c` | `render_production_screen` - compiled to `pscr.bin` |
| `src/ovl_transport.c` | `render_transport_screen` - compiled to `tscr.bin` |
| `asm/ovl_asm.s` | Assembly: `install_trampoline`, `main_to_aux` - AUX RAM copy routines (LOWCODE) |
| `asm/jmptab.s` | 7-entry JMP table at `$080F` - fixed addresses called by overlay binaries |
| `asm/werner.s` | Reserves the HGR segment |
| `include/game.h` | `GameState` struct and all function declarations |
| `include/overlay.h` | Overlay IDs, AUX addresses, trampoline ZP macros |
| `config/apple2-hgr.cfg` | Linker config for the main binary (STARTUP/JMPTAB/LOWCODE/HGR/CODE layout) |
| `config/apple2-ovl.cfg` | Linker config for overlay binaries (raw 2KB at `$8800`, symbols from jump table) |
| `assets/startup.bas` | One-line Applesoft BASIC program: auto-BRUNs IIMPERIALISM at boot |
| `assets/iimperialism.dsk` | ProDOS disk image |
| `Makefile` | Build rules for main binary, overlay binaries, and disk image |
| `build-run.sh` | Clean build + disk update + launch emulator in one step |
| `tools/ac.jar` | AppleCommander - adds binaries to the ProDOS disk image |

Documentation: `docs/MEMORY.md`, `docs/DESIGN.md`, `docs/FONT.md`, `docs/PICTURES.md`, `docs/STRUCTURE.md`

## Prerequisites

**cc65** cross-development package for 6502 systems, and **Java** (for AppleCommander).

### Windows
1. Download the Windows Snapshot (zip) from the [cc65 GitHub releases](https://github.com/cc65/cc65/releases).
2. Extract to a folder (e.g., `C:\cc65`) and add `C:\cc65\bin` to your PATH.
3. Verify: `cl65 --version`

### macOS
```bash
brew install cc65
```

### Linux (Debian/Ubuntu)
```bash
sudo apt-get install cc65
```

## Building

```bash
make disk
```

This compiles the main binary and all three overlay binaries, then copies everything
to `assets/iimperialism.dsk` using AppleCommander. The disk image is ready to run.

To just build without updating the disk:
```bash
make
```

To clean all build artifacts:
```bash
make clean
```

## Running on an Emulator

`assets/iimperialism.dsk` is a **ProDOS** disk image. Load it in any Apple IIe emulator.
The game launches automatically at boot via the `STARTUP` Applesoft program.

If it doesn't auto-launch, at the ProDOS BASIC prompt type:
```
BRUN IIMPERIALISM
```

### Recommended Emulators
- **Cross-Platform:** [MicroM8](https://microm8.com/)
- **Windows:** [AppleWin](https://github.com/AppleWin/AppleWin)
- **macOS:** [Virtual II](https://www.virtualii.com/)
- **Linux:** [LinApple](https://github.com/linappleii/linapple)

## Development Workflow

```bash
./build-run.sh      # clean build -> update disk -> launch emulator
```

When adding a new screen as an overlay, see the step-by-step instructions in `docs/MEMORY.md`
under **"Adding a New Screen as an Overlay"**.
