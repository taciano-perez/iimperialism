# Game Design Document

## Overview

IImperialism is a turn-based economic strategy game for Apple II, built with cc65.
The current playable loop focuses on:

- resource extraction and transport
- production planning and workforce management
- trade expeditions driven by foreign market prices
- naval expansion (traders and frigates)

The player cycles through screen overlays to adjust orders, then advances the turn.
The campaign ends through the Council of Nations: when the player commands enough
votes, the game shows a Taipan-inspired final report with a large score, a boxed
historical rank ladder, and a short judgment sentence.

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
- New trader material costs scale with the trader-capacity multiplier; new
  frigate gun costs scale with the guns-per-frigate multiplier. These costs are
  derived in the Admiralty overlay from the existing cached science-adjusted
  stats, avoiding extra game-state fields.

### Turn Progression

`next_turn()` currently:

1. applies transport orders to warehouse totals
2. applies production conversions
3. clamps production orders to available resources
4. increments turn counter, saturating at `MAX_UINT`

Every tenth turn, the Council of Nations meets. A player victory at the council
branches into the final report screen instead of returning directly to the Main
Screen.

## Screen Flow

Resident main loop (`src/main.c`) dispatches by `state.current_screen`:

- `SCREEN_MAIN` -> resident `render_main_screen()`
- `SCREEN_INDUSTRY` -> `OVL_INDUSTRY` (`ISCR`)
- `SCREEN_TRANSPORT` -> `OVL_TRANSPORT` (`TSCR`)
- `SCREEN_PRODUCTION` -> `OVL_PRODUCTION` (`PSCR`)
- `SCREEN_ADMIRALTY` -> `OVL_ADMIRALTY` (`ASCR`)
- `SCREEN_DIPLOMACY` -> `OVL_DIPLOMACY` (`DSCR`)
- `SCREEN_SCIENCE` -> `OVL_SCIENCE` (`SSCR`)
- `SCREEN_TRADE_EXPEDITION` -> resident `render_trade_market()`, then `OVL_TRADE_EXPEDITION_ACTION`
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
- `OVL_TRADE_EXPEDITION_ACTION` (`TXAC`) for trade expedition input/actions

The trade expedition market screen is resident code in `src/trade_expedition.c`.
This avoids a separate 2 KB overlay file for a small read-only market renderer
while leaving the larger buy/sell action flow in `TXAC`.

### Input Ownership

- `src/ovl_industry.c`: render industry and handle screen switching, `ESC`, and end turn
- `src/ovl_transport.c`: render transport and handle transport input and order-entry flows
- `src/ovl_production.c`: render production and handle top-level production input
- `src/production.c`: resident `production_orders()` helper used by the production overlay
- `src/main.c`: resident startup flow, Main Screen hub, and screen dispatch
- `src/ovl_admiralty.c`: render admiralty and handle trader/warship builds
- `src/ovl_game_menu.c`: handle `ESC` menu actions (new/load/save, return)
- `src/ovl_diplomacy.c`: handle diplomacy input, trade expeditions, and alliance/colony offers
- `src/trade_expedition.c`: resident trade expedition market renderer

## Council Victory And Final Report

The Council of Nations is both the victory check and the endgame presentation.
The player wins when their nation reaches `COUNCIL_VICTORY_VOTES`, currently
`24`, out of `COUNCIL_TOTAL_PROVINCES`, currently `32`.

Vote strength is province-based:

- the player and the two other great powers count as `8` provinces each
- the three minor nations count as `4` provinces each
- the player always votes for themselves
- allied great powers and colonial minor nations vote for the player
- other great powers vote for themselves
- unaligned minor nations abstain

On victory, `src/ovl_council_nations.c` first shows the normal Council result
briefly, then replaces it with `FINAL REPORT TO THE CROWN`. The report blends the
game's imperial tone with the concise ranking-screen rhythm of Apple II `Taipan!`.

The final report currently shows:

- net treasury
- sea power as `frigates * guns_per_frigate`
- merchant fleet as trader count and total capacity
- foreign relations as the number of friendly provinces that voted for the player
- winning turn count
- an inverted `Your score is ...` line
- a boxed five-rank table with the achieved rank printed inverted
- a two-line humorous judgment for the achieved rank

Sea power, merchant capacity, and science remain important gameplay values, but
they are report/context values only; they are not final score inputs.

The screen deliberately remains text-first. It does not introduce a new overlay
or new bitmap asset.

### Final Score

The score is calculated in resident logic and presented by the Council overlay.
All tuning values live as named constants in `include/game.h`, not as literals in
the overlay.

The score deliberately considers only three factors: diplomacy, speed, and
treasury. Navy, merchant marine, and science no longer contribute directly.

The score inputs are normalized to `0..100`:

- diplomacy: friendly provinces above the 24-vote victory threshold, divided by
  the remaining possible friendly provinces
- treasury: threshold bands from `SCORE_TREASURY_1` through `SCORE_TREASURY_5`
- speed: threshold bands from `SCORE_SPEED_TURN_1` through `SCORE_SPEED_TURN_5`

Category weights:

- diplomacy: `SCORE_WEIGHT_DIPLOMACY`
- treasury: `SCORE_WEIGHT_TREASURY`
- speed: `SCORE_WEIGHT_SPEED`

The weighted score is divided by `100` and then multiplied by
`SCORE_SCALE_FACTOR`. Speed uses `SCORE_SPEED_SCORE_*` bands selected by
comparing the current `turn_number` to `SCORE_SPEED_TURN_*`. The real campaign
turn count starts at turn `1`, so no offset is applied.

The target result is a large Taipan-style number, with exceptional wins reaching
`50,000` or more while still fitting in an `unsigned int`.

### Historical Rank Ladder

The final score maps to a five-step rank ladder:

| Score range | Rank |
|-------------|------|
| `50,000 and over` | Queen Victoria |
| `35,000 to 49,999` | Otto von Bismarck |
| `20,000 to 34,999` | Napoleon III |
| `8,000 to 19,999` | Charles X |
| `less than 8,000` | Ferdinand VII |

The rank table displays rank first and score range second, matching the Taipan
reference. The achieved rank is printed with inverted text.

Each rank has a two-line judgment:

- Queen Victoria: `You founded an Empire` / `where the sun never sets.`
- Otto von Bismarck: `Diplomacy forged with` / `an iron will.`
- Napoleon III: `You reigned confidently,` / `though not always wisely.`
- Charles X: `Your court looked grander` / `than your results.`
- Ferdinand VII: `Your creditors remember you` / `more vividly than your subjects.`

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
- per-turn ledger totals (`trade_expenses`, `trade_revenue`, `turn_booty`) stored
  as `unsigned int` so they do not wrap during active trade/battle turns

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

### Overflow Guard Status

The current build keeps only guards that fit while preserving at least three
free ProDOS blocks on the packaged floppy:

- resource production uses saturating warehouse adds
- `turn_number` saturates at `MAX_UINT`
- per-turn ledger totals are `unsigned int`
- battle bounty is calculated as `unsigned int`
- production orders and transport orders are capped before assigning into
  `unsigned char` fields
- worker and wagon builds are capped against their destination storage limits

The following guards were measured but not kept because they either overflowed
their 2KB overlay or required resident helper code that reduced disk free space
below three ProDOS blocks:

- trade buy cap by remaining warehouse room
- saturating treasury gains on trade sells and battle bounty
- admiralty build caps for `traders` and `frigates`

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
- add a post-victory play-again prompt
