# Game Design Document

## Overview

IImperialism is a turn-based economic strategy game for Apple II, built with cc65.
The current playable loop focuses on:

- resource extraction and transport
- production planning and workforce management
- trade expeditions driven by foreign market prices
- naval expansion (traders and frigates)

The player cycles through screen overlays to adjust orders, then advances the turn.

## Current Gameplay Model

### Resource Layers

- `GameState.resources[]` stores warehouse inventory, indexed by `RESOURCE_*`
  constants in `include/game.h`.
- Raw: `RESOURCE_TIMBER`, `RESOURCE_WOOL`, `RESOURCE_IRON`, `RESOURCE_COAL`
- Processed: `RESOURCE_LUMBER`, `RESOURCE_FABRIC`, `RESOURCE_STEEL`
- Finished: `RESOURCE_FURNITURE`, `RESOURCE_CLOTHES`, `RESOURCE_TOOLS`, `RESOURCE_GUNS`

### Money and Trade

- `GameState.money` stores the treasury used for market purchases.
- Foreign nations advertise buy/sell prices through `export_prices[]` and
  `import_prices[]`.
- Trade expeditions are limited by both `remaining_turn_capacity` and cash on hand.
- Buying spends money at the nation's export price; selling earns money at the
  nation's import price.

### Capacity and Workforce

- Wagons limit transport throughput
- Workers limit production throughput
- Traders and frigates are built in admiralty workflows

### Turn Progression

`next_turn()` currently:

1. applies transport orders to warehouse totals
2. applies production conversions
3. clamps production orders to available resources
4. increments turn counter

## Screen Flow

Resident main loop (`src/main.c`) dispatches by `state.current_screen`:

- `SCREEN_INDUSTRY` -> `OVL_INDUSTRY` (`ISCR`)
- `SCREEN_TRANSPORT` -> `OVL_TRANSPORT` (`TSCR`)
- `SCREEN_PRODUCTION` -> `OVL_PRODUCTION` (`PSCR`)
- `SCREEN_ADMIRALTY` -> `OVL_ADMIRALTY` (`ASCR`)

Sub-flows are also overlays:

- `OVL_ADMIRALTY_TRADER` (`ATRD`) for trader construction
- `OVL_ADMIRALTY_WARSHIP` (`AWRS`) for frigate construction
- `OVL_TRADE_EXPEDITION` (`TEXP`) for the trade expedition market screen
- `OVL_TRADE_EXPEDITION_ACTION` (`TXAC`) for trade expedition input/actions

### Input Handlers

- `src/industry.c`: switch to transport/production/admiralty, or end turn
- `src/transport.c`: edit transport orders, build wagons, return
- `src/production.c`: edit production orders, train workers, return
- `src/admiralty.c`: build traders/warships, return
- `src/diplomacy.c`: launch trade expedition flow, return

## Data Model

Global `GameState state` (see `include/game.h`) includes:

- resource inventories in `resources[RESOURCE_COUNT]`
- treasury (`money`)
- province yields
- transport orders and available wagons
- production orders and available workers
- navy counts (`traders`, `frigates`)
- metadata (`turn_number`, `nation_name`, `current_screen`)
- turn-specific trade state (`remaining_turn_capacity`)

Persistence is handled in `src/gamestate.c` via `GAME.DATA`.

## Runtime Architecture

- Main binary is resident code + LOWCODE + shared data.
- Overlays are 2KB binaries loaded into `$8800` on demand.
- Overlay calls to resident helpers go through fixed jump table entries (`asm/jmptab.s`).

See `docs/MEMORY.md` for exact map and loader flow.

## Boot and Distribution

The floppy image (`assets/iimperialism.dsk`) autoboots through:

1. `PRODOS`
2. `IIMP.SYSTEM` (loader)
3. `IIMPERIALISM` (main BIN)

See `docs/FLOPPY.md` for details.

## Build Targets

- `make` builds main binary, overlays, and loader artifacts.
- `make disk` updates the ProDOS disk image.
- `make clean` removes build artifacts.

On some Windows environments, `make SHELL=cmd disk` is required.

## Near-Term Roadmap

- science screen
- diplomacy screen
- trade voyage screens
- menu, save/load/quit flow, and endgame/retirement flow
