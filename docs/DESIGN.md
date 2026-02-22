# Game Design Document

## Overview

This is an economic strategy game for the Apple II computer, inspired by the classic "Imperialism" series. The player manages a nation's economy, collecting raw materials, producing goods, and managing resources through a turn-based system.

## Target Platform

- **Hardware**: Apple II series (Apple II, II+, IIe, IIc, IIgs)
- **Compiler**: CC65 cross-compiler toolchain
- **Graphics**: Hi-Res graphics mode (280x192, 6 colors)
- **Display**: 40-column text mode for UI elements

## File Structure

### Core Game Files

#### `main.c`
**Purpose**: Main game loop and screen rendering
- Entry point for the game
- Contains the main game loop that handles input and rendering
- `render_economy_screen()` - Renders the main economy view showing:
  - Player and nation name header
  - Warehouse inventory (11 resource types)
  - Transport orders (4 raw material types)
  - Production orders (7 processed/finished good types)
  - Advisor portrait and prompt
- Input handling for:
  - `T` - Transport orders menu
  - `P` - Production orders menu
  - `E` - End turn
  - `ESC` - Exit game

#### `game.h`
**Purpose**: Main header file with shared definitions
- Picture index constants (`WISEMAN_PORTRAIT`, `MILITARY_PORTRAIT`)
- `GameState` struct definition (see Data Structures section)
- External declaration of global `state` variable
- Function prototypes for all modules:
  - UI functions (ui.c)
  - Game logic functions (logic.c)
  - Save/load functions (gamestate.c)

#### `logic.c`
**Purpose**: Game logic and state management
- `init_game()` - Initializes game state with default values:
  - Starting resources (10 timber, 10 cotton, 5 iron, 5 coal)
  - Default transport orders (5 timber, 5 cotton, 1 iron, 1 coal per turn)
  - Default production orders (2 lumber, 2 fabric, 1 steel, 1 furniture, 1 clothes per turn)
  - Turn number set to 1
  - Player name: "Taciano", Nation: "Haxaco"
- `next_turn()` - Processes end-of-turn updates:
  - Adds transported raw materials to warehouse
  - Adds produced goods to warehouse
  - Increments turn counter

#### `gamestate.c`
**Purpose**: Save/load game functionality
- `save_game(const GameState* state)` - Saves game to `GAME.DATA`
  - Writes magic number (0x4947 - "IG") for validation
  - Writes version number for compatibility checking
  - Writes entire GameState struct as binary data
  - Returns 0 on success, -1 on failure
- `load_game(GameState* state)` - Loads game from `GAME.DATA`
  - Validates magic number and version
  - Reads GameState struct from file
  - Returns 0 on success, -1 on failure

### UI and Graphics Files

#### `ui.c`
**Purpose**: User interface and graphics rendering
- `ui_init()` - Initializes graphics mode
- `ui_exit()` - Exits graphics mode and returns to text mode
- `clear_screen()` - Clears the display
- `print(x, y, text)` - Prints text at specified character coordinates
- `print_right_aligned(x, y, text)` - Prints text right-aligned to specified x position
- `print_int_right_aligned(x, y, value)` - Converts unsigned int to string and prints right-aligned
- `draw_picture_at(picture_index, x, y)` - Draws pre-defined pictures (portraits, icons)
- `box(x1, y1, x2, y2)` - Draws a box border

#### `werner.s`
**Purpose**: Assembly language optimizations
- Low-level routines for performance-critical operations
- Assembly language file (`.s` extension)

#### `font.h`
**Purpose**: Custom font definitions
- Custom font data for text rendering
- See `FONT.md` for detailed documentation

#### `pictures.h`
**Purpose**: Picture/sprite definitions
- Bitmap data for portraits and icons
- Currently includes advisor portraits
- See `PICTURES.md` for detailed documentation

#### `icons.h`
**Purpose**: Icon definitions
- Smaller graphical elements (resource icons, UI elements)

### Build Configuration

#### `Makefile`
**Purpose**: Build system configuration
- Source files: `main.c`, `ui.c`, `werner.s`, `gamestate.c`, `logic.c`
- Target: `iimperialism` (executable)
- Compiler: `cl65` (CC65 compiler/linker)
- Flags: `-t apple2` (target Apple II), `-C apple2-hgr.cfg` (use HGR config), `-Oirs` (optimizations)
- Targets:
  - `all` - Build the game
  - `clean` - Remove object files and executable

#### `apple2-hgr.cfg`
**Purpose**: Linker configuration for Hi-Res graphics mode
- Memory layout for Apple II with Hi-Res graphics
- Defines code and data segment placement

#### `apple2-raw.cfg`
**Purpose**: Alternative linker configuration
- Raw binary output format

## Data Structures

### GameState Structure

The `GameState` struct (defined in `game.h`) is the core data structure that holds all game state.

**Global Instance**: `state` - Declared in `main.c`, externally visible via `game.h`

## Game Flow

### Startup Sequence

1. `main()` called (main.c:93)
2. `init_game()` - Initialize game state with default values (logic.c:4)
3. `ui_init()` - Initialize graphics mode (ui.c)
4. Enter main game loop

### Main Game Loop

The game loop (main.c:99-124) continuously:

1. `render_economy_screen()` - Draw the current state
2. `cgetc()` - Wait for keypress
3. Process input:
   - **T**: Switch to transport orders screen (TODO)
   - **P**: Switch to production orders screen (TODO)
   - **E**: Call `next_turn()` to advance the game
   - **ESC**: Exit game

### Turn Processing

When player presses 'E' (End Turn):

1. `next_turn()` called (logic.c:39)
2. Raw materials added based on transport orders
3. Processed/finished goods added based on production orders
4. Turn counter incremented
5. Control returns to main loop
6. Screen re-renders with updated values

### Save/Load Flow

**Saving**:
1. Call `save_game(&state)`
2. Opens `GAME.DATA` for binary write
3. Writes magic number (0x4947) and version (1)
4. Writes entire GameState struct
5. Returns 0 on success

**Loading**:
1. Call `load_game(&state)`
2. Opens `GAME.DATA` for binary read
3. Validates magic number and version
4. Reads GameState struct
5. Returns 0 on success, -1 if file not found or corrupted

## Resource Production Chain

### Tier 0: Raw Materials (Transported)
- **Timber** → Used to produce Lumber
- **Cotton** → Used to produce Fabric
- **Iron** + **Coal** → Used to produce Steel

### Tier 1: Processed Materials
- **Lumber** → Used to produce Furniture
- **Fabric** → Used to produce Clothes
- **Steel** → Used to produce Tools or Cannons

### Tier 2: Finished Goods
- **Furniture** → Final product
- **Clothes** → Final product
- **Tools** → Final product
- **Cannons** → Final product (military)

## Screen Layout

### Economy Screen (40 columns x 24 rows)

```
Row 0:  President <name> of <nation>     <date>
Row 2:  0123456789... (debug ruler)
Row 3:  +----------------------------------------+
        | Warehouse                             |
Row 4:  | Timber:  #### Lumber:  ### Furniture: ####
Row 5:  | Cotton:  #### Fabric:  ### Clothes:   ####
Row 6:  | Iron:    #### Steel:   ### Tools:     ####
Row 7:  | Coal:    ####              Cannons:   ####
Row 8:  +----------------------------------------+
Row 9:  +----------------------------------------+
        | Transport &    Production Orders      |
Row 10: | Timber: #### Lumber:    ### Furniture: ####
Row 11: | Cotton: #### Fabric:    ### Clothes:   ####
Row 12: | Iron:   #### Steel:     ### Tools:     ####
Row 13: | Coal:   ####                Cannons:   ####
Row 14: +----------------------------------------+
Row 20: [Advisor Portrait]
        What are your orders, sir?
        Change Transport, Production,
        or End turn?
```

## Future Enhancements

### Planned Features
- Transport orders editing screen
- Production orders editing screen
- Resource consumption mechanics (production requires raw materials)
- Economy simulation (market prices, trade)
- Multiple regions/provinces
- Military units and combat
- Diplomacy system
- Victory conditions

### Technical TODOs
- Add error handling for file I/O operations
- Implement UI feedback for save/load operations
- Add input validation for order editing
- Implement resource consumption in production
- Add sound effects (mockingboard support)
- Optimize rendering (only update changed areas)

## Development Notes

### Memory Constraints
- Apple II has 48KB-64KB RAM
- Hi-Res graphics uses ~16KB
- Keep code size minimal with `-Oirs` optimization
- Use `unsigned int` (16-bit) for resource counts (max 65535)

### Build Commands
```bash
make clean    # Remove build artifacts
make          # Compile and link
make all      # Same as make
```

### Testing on Emulator
The game produces an auto-bootable disk image (`iimperialism.dsk`) that can be loaded directly into Apple II emulators like AppleWin or MAME.

### Code Style
- Use clear, descriptive function names
- Comment major sections with `/* ===== */` headers
- Keep functions focused and small
- Prefer clarity over cleverness (this is for a 6502!)

## References

- **CC65 Documentation**: https://cc65.github.io/
- **Apple II Technical Reference**: Apple II Reference Manual
- **Original Inspiration**: Imperialism (1997) and Imperialism II (1999)
