# IImperialism - an Apple II Strategy Game

A strategy game for the Apple II, written in C using the **cc65** compiler
and its Tiny Graphics Interface (TGI) for Hi-Res Graphics (HGR) mode.

This project is inspired by Imperialism and Taipan!. It is a turn-based resource
management game where players manage a supply chain from raw materials through
production stages to finished goods while managing cash and foreign trade prices
that shift with each nation's current relations.
Science research unlocks higher trader capacity and warship firepower over time,
with new shipbuilding costs scaling to match those stronger vessels.

## Overview

The game runs within the Apple II's main 64KB memory map. It uses a custom memory layout to fit
within the Apple II's constraints:

- Most screen renderers are compiled as standalone 2KB overlays (`ISCR`, `PSCR`,
  `TSCR`, `ASCR`, `DSCR`, `TXAC`, `BSCR`, `SSCR`, `MENU`, `CNSL`) loaded on demand.
- The trade expedition market screen is resident code; only the buy/sell action
  flow remains in the `TXAC` overlay.
- The industry, transport, and production overlays now own their own input loops.
- The industry overlay also owns the ledger sub-screen.
- The transport screen is fully overlay-local.
- The production screen still calls resident `production_orders()` through the jump
  table to preserve overlay space.
- The Council of Nations overlay owns the endgame flow, including the final
  report, score display, and historical rank table.
- The final score now uses only three factors: diplomacy beyond the bare
  24-vote victory, speed of victory, and treasury.
- Overlay binaries are loaded at runtime with direct ProDOS MLI `OPEN` / `READ` /
  `CLOSE` calls instead of `stdio`.
- Save/load also use direct ProDOS MLI calls instead of `fopen()` / `fread()` /
  `fwrite()`.
- On first launch, the splash screen waits for `ESC`, uses that human-timed delay
  to seed gameplay randomness, and then prompts for a nation name (up to 10 chars).
- After a new game starts, play enters the resident Main Screen, which acts as the
  top-level hub for Industry, Science, Admiralty, Diplomacy, end-turn, and the
  transient game menu.
- Pressing `ESC` opens the `MENU` overlay for new/load/save actions.
- Choosing `New Game` from the menu also re-prompts for the nation name.
- The resident helper split is now:
  - `JMPTAB` at `$080F-$0871` for overlay-callable entry points
  - `LOWCODE` at `$0872-$1FFF` for compact main-RAM helpers such as the HGR text blitters
  - `LC` at `$D400-$DD79` for `src/ui.c` UI code placed in the Language Card
- Disk autoboot uses ProDOS `SYS` loader `IIMP.SYSTEM` to launch `IIMPERIALISM`
  directly (no `BASIC.SYSTEM` dependency).

See `docs/MEMORY.md` for memory and overlay details.
See `docs/FLOPPY.md` for floppy contents and autoboot behavior.

## Diplomacy Relations

Foreign nation relations are stored as numeric values in the game state and shown
in the diplomacy screen as text using these ranges:

- `0-49`: Bad
- `50-99`: Poor
- `100-149`: Fair
- `150-199`: Good
- `200-254`: Great
- `255`: Ally for great powers, Colony for minor nations

Trade prices also scale with these relation tiers: stronger relations make a
nation's exports cheaper to buy and its imports more profitable to sell into.

## File Structure

| File | Purpose |
|------|---------|
| `src/main.c` | Main loop, startup/new-game flow, and screen dispatch |
| `src/production.c` | Resident `production_orders()` helper used by `pscr.bin` |
| `src/ui.c` | UI primitives (`print`, `print_inverted`, `box`, `clear_screen`, `scan_uint`, `scan_text`, etc.); code is linked into the Language Card (`LC`) |
| `src/ui_buffers.c` | Shared scratch UI buffer storage |
| `src/logic.c` | `init_game()`, `next_turn()`, final score calculation, and rank selection |
| `src/gamestate.c` | Shared `GameState` declarations |
| `src/overlay.c` | `init_overlays()`, `run_overlay()` |
| `src/trade_expedition.c` | Resident diplomacy trade expedition market renderer |
| `src/ovl_industry.c` | Industry screen overlay and ledger sub-screen (`iscr.bin`) |
| `src/ovl_production.c` | Production screen overlay and top-level input loop (`pscr.bin`) |
| `src/ovl_transport.c` | Transport screen overlay, input loop, and transport actions (`tscr.bin`) |
| `src/ovl_admiralty.c` | Admiralty screen and build flow overlay (`ascr.bin`) |
| `src/ovl_diplomacy.c` | Diplomacy screen overlay (`dscr.bin`) |
| `src/ovl_trade_expedition_action.c` | Diplomacy trade expedition action overlay (`txac.bin`) |
| `src/ovl_science.c` | Science screen overlay (`sscr.bin`) |
| `src/ovl_game_menu.c` | Game menu overlay and save/load flow (`menu.bin`) |
| `src/ovl_council_nations.c` | Council of Nations overlay and Taipan-inspired final report (`cnsl.bin`) |
| `asm/prodos_overlay_load.s` | Resident ProDOS MLI overlay loader (`OPEN` / `READ` / `CLOSE`) |
| `asm/prodos_gamestate_io.s` | Menu-overlay ProDOS MLI save/load helper for `GAME.DATA` |
| `asm/ovl_industry_entry.s` | Fixed entry stub for industry overlay |
| `asm/ovl_diplomacy_entry.s` | Fixed entry stub for diplomacy overlay |
| `asm/ovl_production_entry.s` | Fixed entry stub for production overlay |
| `asm/ovl_transport_entry.s` | Fixed entry stub for transport overlay |
| `asm/ovl_trade_expedition_action_entry.s` | Fixed entry stub for trade expedition action overlay |
| `asm/ovl_science_entry.s` | Fixed entry stub for science overlay |
| `asm/ovl_game_menu_entry.s` | Fixed entry stub for game menu overlay |
| `asm/ovl_council_nations_entry.s` | Fixed entry stub for council overlay |
| `asm/text_hgr.s` | Aligned opaque HGR text blitters used by `print()`, `print_bold()`, and `print_inverted()` |
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
`docs/OPTIMIZE_CODE.md`, `docs/OPTIMIZE_REFACTOR.md`

## Prerequisites

- **cc65** cross-development package
- **Java** (for AppleCommander)

## Runtime Requirements

The current build targets a **64 KB Apple II family machine with ProDOS support**.

Supported in principle:

- Apple IIe
- Apple IIc
- Apple IIgs running Apple II compatible ProDOS software
- Apple II / Apple II Plus class machines only if they have enough RAM and hardware support to run ProDOS correctly

Not supported:

- 48 KB Apple II configurations
- Apple II / Apple II Plus / Europlus machines without the RAM expansion needed to reach 64 KB

Why:

- the game uses the main 64 KB Apple II memory map
- it relies on ProDOS 8 boot/loading and direct ProDOS MLI calls
- it uses HGR graphics mode and loads 2 KB overlays into main RAM at runtime

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
- `dscr.bin` now pulls most diplomacy UI text from resident code through
  `get_diplomacy_string()` in `JMPTAB`, which keeps overlay `RODATA` below 2 KB
- `cnsl.bin` pulls final-report strings and score helpers from resident code
  through `JMPTAB`, keeping the Council overlay below the 2 KB limit
- resident UI text now uses direct HGR byte writes from `asm/text_hgr.s`
- `src/ui.c` itself is linked into the Language Card, while the blitters remain in
  main-RAM `LOWCODE`
- visual verification in the emulator is still required after text-rendering changes

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

Core Features
- Balance game for all stages (beginning, mid, and end)

Performance/Maintanability Improvements
- Rename frigate to warship
- Attempt to replace ProDOS by RWTS

Discarded ideas (difficult to squeeze in without requiring extra floppies):
- Check memory size, if >64K, copy overlays from disk into memory at load time
- Random events (positive and negative) at turn's end
- Map screen
- Land battles

Packaging goodies
- Manual
- Floppy Sticker
