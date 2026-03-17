# IImperialism - an Apple II Strategy Game

An early-stage strategy game for the Apple II, written in C using the **cc65** compiler
and its Tiny Graphics Interface (TGI) for Hi-Res Graphics (HGR) mode.

This project is inspired by Imperialism and Taipan!. It is a turn-based resource
management game where players manage a supply chain from raw materials through
production stages to finished goods while managing cash and foreign trade prices.
Science research unlocks higher trader capacity and warship firepower over time.

## Overview

The game runs within the Apple II's main 64KB memory map. It uses a custom memory layout to fit
within the Apple II's constraints:

- Screen renderers are compiled as standalone 2KB overlays (`ISCR`, `PSCR`, `TSCR`,
  `ASCR`, `DSCR`, `TEXP`, `TXAC`, `BSCR`, `SSCR`, `MENU`) loaded on demand.
- Overlay binaries are loaded at runtime with direct ProDOS MLI `OPEN` / `READ` /
  `CLOSE` calls instead of `stdio`.
- UI primitives and core game logic are kept in **LOWCODE** (`$0824-$1FFF`) below
  the HGR screen to preserve resident code space.
- Disk autoboot uses ProDOS `SYS` loader `IIMP.SYSTEM` to launch `IIMPERIALISM`
  directly (no `BASIC.SYSTEM` dependency).

See `docs/MEMORY.md` for memory and overlay details.
See `docs/FLOPPY.md` for floppy contents and autoboot behavior.

## Diplomacy Relations

Foreign nation relations are stored as numeric values in the game state and shown
in the diplomacy screen as text using these ranges:

- `0-49`: Terrible
- `50-99`: Bad
- `100-149`: Neutral
- `150-199`: Good
- `200+`: Excellent

## File Structure

| File | Purpose |
|------|---------|
| `src/main.c` | Main loop and screen dispatch |
| `src/ui.c` | UI primitives (`print`, `box`, `clear_screen`, etc.) |
| `src/ui_buffers.c` | Shared scratch UI buffer storage |
| `src/logic.c` | `init_game()`, `next_turn()` |
| `src/gamestate.c` | Save/load game state (`GAME.DATA`) |
| `src/overlay.c` | `init_overlays()`, `run_overlay()` |
| `src/ovl_industry.c` | Industry screen overlay (`iscr.bin`) |
| `src/ovl_production.c` | Production screen overlay (`pscr.bin`) |
| `src/ovl_transport.c` | Transport screen overlay (`tscr.bin`) |
| `src/ovl_admiralty.c` | Admiralty screen and build flow overlay (`ascr.bin`) |
| `src/ovl_diplomacy.c` | Diplomacy screen overlay (`dscr.bin`) |
| `src/ovl_trade_expedition.c` | Diplomacy trade expedition market overlay (`texp.bin`) |
| `src/ovl_trade_expedition_action.c` | Diplomacy trade expedition action overlay (`txac.bin`) |
| `src/ovl_science.c` | Science screen overlay (`sscr.bin`) |
| `src/ovl_game_menu.c` | Game menu overlay (`menu.bin`) |
| `asm/prodos_overlay_load.s` | Resident ProDOS MLI overlay loader (`OPEN` / `READ` / `CLOSE`) |
| `asm/ovl_diplomacy_entry.s` | Fixed entry stub for diplomacy overlay |
| `asm/ovl_trade_expedition_entry.s` | Fixed entry stub for trade expedition market overlay |
| `asm/ovl_trade_expedition_action_entry.s` | Fixed entry stub for trade expedition action overlay |
| `asm/ovl_science_entry.s` | Fixed entry stub for science overlay |
| `asm/text_hgr.s` | Aligned opaque HGR text blitter used by `print()` |
| `asm/jmptab.s` | Resident jump table used by overlays |
| `asm/werner.s` | Reserves HGR segment |
| `asm/loader/loader.s` | Vendored cc65 loader source (patched for fixed target BIN) |
| `asm/loader/loader.cfg` | Linker config for loader system file |
| `include/game.h` | `GameState` and shared declarations |
| `include/ui_buffers.h` | Shared UI buffer declarations |
| `include/overlay.h` | Overlay IDs, filenames, and loader constants |
| `config/apple2-hgr.cfg` | Main linker config |
| `config/apple2-ovl.cfg` | Overlay linker config template |
| `build/apple2-ovl.cfg` | Generated overlay linker config with current `_state` address |
| `assets/iimperialism.dsk` | ProDOS disk image |
| `Makefile` | Build rules for main binary, overlay entry stubs, overlays, loader, and disk |
| `build-run.sh` | Build + disk update + emulator launch helper |
| `tools/ac.jar` | AppleCommander utility |

Documentation: `docs/FLOPPY.md`, `docs/MEMORY.md`, `docs/DESIGN.md`,
`docs/FONT.md`, `docs/PICTURES.md`, `docs/STRUCTURE.md`,
`docs/OPTIMIZE_CODE.md`

## Prerequisites

- **cc65** cross-development package
- **Java** (for AppleCommander)

### Windows

1. Download a Windows snapshot from [cc65 releases](https://github.com/cc65/cc65/releases).
2. Extract it (for example `C:\cc65`) and add its `bin` folder to `PATH`.
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

This builds the main binary, all overlays, and loader system file, then updates
`assets/iimperialism.dsk`.

On some Windows setups, use:

```bash
make SHELL=cmd disk
```

To build without updating the disk image:

```bash
make
```

To clean artifacts:

```bash
make clean
```

To inspect resident memory usage and overlay occupancy:

```bash
make memory-usage
```

Overlay note:

- `config/apple2-ovl.cfg` is a template
- the build generates `build/apple2-ovl.cfg` from `build/iimperialism.map`
- overlays are linked against the generated file so `_state` stays in sync with the
  current resident layout
- resident UI text now uses direct HGR byte writes from `asm/text_hgr.s`, so visual
  verification in the emulator is still required after text-rendering changes

In your setup (Git Bash on Windows), use:

```bash
make memory-usage
```

## Running on an Emulator

`assets/iimperialism.dsk` is a ProDOS disk image. Load it in an Apple IIe emulator.

Autoboot path:

- ProDOS starts `IIMP.SYSTEM`
- `IIMP.SYSTEM` loads and jumps to `IIMPERIALISM`

If you need to launch manually from a ProDOS BASIC prompt:

```text
BRUN IIMPERIALISM
```

### Recommended Emulators

- **Cross-platform:** [MicroM8](https://microm8.com/)
- **Windows:** [AppleWin](https://github.com/AppleWin/AppleWin)
- **macOS:** [Virtual II](https://www.virtualii.com/)
- **Linux:** [LinApple](https://github.com/linappleii/linapple)

## Development Workflow

```bash
./build-run.sh
```

This runs a build, updates the disk image, and launches the emulator.

When adding a new screen as an overlay, see `docs/MEMORY.md` under
"Adding a New Overlay". After any resident-code size change, do a full rebuild so
the generated `build/apple2-ovl.cfg` picks up the current `_state` address before
overlays are relinked.

## TODO

- Finish battle screen 
  - Randomize # of attackers based on turn
  - 5% chance of enemy hitting traders
  - capture booty
  - introduce enemy powers and not only pirates
  - add sound effects
- Expand the science tree with wagon capacity improvements
- Refactor for saving memory
  - Unless we need it for country name, remove cgetc_at from ui.c
- Add main menu
- Add retirement/end screen
- Add splash screen at startup and seed random number gen
- Randomize country names and their exports/imports
- On ovl_diplomacy, lower the chance of a battle (currently at 50% for testing)
- Decrease the relationship status per turn
- Adjust the initial money amount
- Admiralty improvement: increase cost of traders and warships according to science level
- Attempt to merge overlays when each is less than 1KB to reclaim floppy space
- Add event alerts (e.g. war, degrading relations, random events) at turn's end, use overlay for event strings if needed
