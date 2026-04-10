# Game Design Document

## Overview

IImperialism is a turn-based economic strategy game for Apple II, built with cc65.
The current playable loop focuses on:

- resource extraction and transport
- production planning and workforce management
- trade expeditions driven by foreign market prices
- naval expansion (traders and frigates)

The player cycles through screen overlays to adjust orders, then advances the turn.

## Startup Flow

On cold start, the player sees a splash screen and must press `ESC` to continue.
The delay before that keypress is mixed into the RNG seed to avoid deterministic
market and battle rolls. After the splash, the player is prompted to enter a
nation name of up to 10 characters before the initial game state is created.

The same nation-naming prompt is also reused by the game menu's `New Game` action.

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
- Those prices are regenerated from base commodity values with relation-based
  adjustments, so better relations improve trade terms.
- Trade expeditions are limited by both `remaining_turn_capacity` and cash on hand.
- Buying spends money at the nation's export price; selling earns money at the
  nation's import price.

### Capacity and Workforce

- Wagons limit transport throughput
- Workers limit production throughput
- Traders and frigates are built in admiralty workflows
- Science research increases `capacity_per_trader` and `guns_per_frigate`,
  which affect trade-expedition capacity and battle firepower

### Turn Progression

`next_turn()` currently:

1. applies transport orders to warehouse totals
2. applies production conversions
3. clamps production orders to available resources
4. increments turn counter

## Screen Flow

Resident main loop (`src/main.c`) dispatches by `state.current_screen`:

- `SCREEN_MAIN` -> resident `render_main_screen()`
- `SCREEN_INDUSTRY` -> `OVL_INDUSTRY` (`ISCR`)
- `SCREEN_TRANSPORT` -> `OVL_TRANSPORT` (`TSCR`)
- `SCREEN_PRODUCTION` -> `OVL_PRODUCTION` (`PSCR`)
- `SCREEN_ADMIRALTY` -> `OVL_ADMIRALTY` (`ASCR`)
- `SCREEN_DIPLOMACY` -> `OVL_DIPLOMACY` (`DSCR`)
- `SCREEN_SCIENCE` -> `OVL_SCIENCE` (`SSCR`)
- `SCREEN_TRADE_EXPEDITION` -> `OVL_TRADE_EXPEDITION` then `OVL_TRADE_EXPEDITION_ACTION`
- `SCREEN_BATTLE` -> resident battle prelude, then `OVL_BATTLE` (`BSCR`)
- `SCREEN_COUNCIL_NATIONS` -> `OVL_COUNCIL_NATIONS` (`CNSL`)
- `SCREEN_LEDGER` -> `OVL_INDUSTRY` (`ISCR`) ledger sub-screen

The Main Screen is the player's hub between turns. It shows the nation name,
turn number, funds, and an advisor prompt. From there the player can choose:

- `I` for Industry
- `S` for Science
- `A` for Admiralty
- `F` for Diplomacy / Foreign Office
- `E` to advance to the next turn
- `ESC` to open the game menu overlay

`ESC` from the Main Screen opens the game menu overlay as a transient flow and
then returns to the Main Screen unless the menu action changes game state. For
industry, transport, and production, `ESC` is handled inside the overlay's own
input loop.

Sub-flows are also overlays:

- `OVL_GAME_MENU` (`MENU`) for new/load/save actions from `ESC`
- `OVL_TRADE_EXPEDITION` (`TEXP`) for the trade expedition market screen
- `OVL_TRADE_EXPEDITION_ACTION` (`TXAC`) for trade expedition input/actions

### Input Ownership

- `src/ovl_industry.c`: render industry and handle screen switching, `ESC`, and end turn
- `src/ovl_transport.c`: render transport and handle transport input and order-entry flows
- `src/ovl_production.c`: render production and handle top-level production input
- `src/production.c`: resident `production_orders()` helper used by the production overlay
- `src/main.c`: resident startup flow, Main Screen hub, and screen dispatch
- `src/ovl_admiralty.c`: render admiralty and handle trader/warship builds
- `src/ovl_game_menu.c`: handle `ESC` menu actions (new/load/save, return)
- `src/ovl_diplomacy.c`: handle diplomacy input, trade expeditions, and alliance/colony offers

## Data Model

Global `GameState state` (see `include/game.h`) includes:

- resource inventories in `resources[RESOURCE_COUNT]`
- treasury (`money`)
- province yields
- transport orders and available wagons
- production orders and available workers
- navy counts (`traders`, `frigates`)
- derived navy stats (`capacity_per_trader`, `guns_per_frigate`)
- science progression (`science_level`)
- metadata (`turn_number`, `nation_name`, `current_screen`)
- turn-specific trade state (`remaining_turn_capacity`)

Persistence is owned entirely by the game menu overlay and uses a local
direct-ProDOS MLI helper for `GAME.DATA` rather than `stdio`.

## Runtime Architecture

- Main binary is resident code plus three important helper regions:
  - `JMPTAB` in main RAM for fixed overlay-callable entry points
  - `LOWCODE` in main RAM for compact resident helpers and core logic
  - `LC` in Language Card RAM for `src/ui.c` UI code
- Overlays are 2KB binaries loaded into `$8800` on demand.
- Overlay calls to resident helpers go through jump table entries in
  `asm/jmptab.s`, with matching symbol addresses exported from
  `config/apple2-ovl.cfg`.
- The diplomacy overlay now uses a resident `get_diplomacy_string()` helper through
  `JMPTAB` so its larger UI text set does not consume local overlay `RODATA`.
- After changing the jump table or resident layout, rebuild overlays so the
  generated `build/apple2-ovl.cfg` picks up the current `_state` address and
  resident helper entry points.

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

- expand diplomacy depth beyond the current trade-expedition entry flow
- add more trade-voyage and battle variety
- add endgame/retirement flow
