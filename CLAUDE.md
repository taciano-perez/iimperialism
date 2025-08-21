# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an Apple II game written in Applesoft BASIC, inspired by Imperialism and Taipan! It's a turn-based resource management game where players manage a supply chain from raw materials (Timber/Cotton) through production stages to finished goods (Furniture/Clothes).

## Development Environment

This project is designed to run on Apple II emulators or actual Apple II hardware. The main game file is `iimperialism.bas` which contains Applesoft BASIC code.

To run the game:
- Load `iimperialism.bas` into an Apple II emulator or system
- Type `RUN` to start the game

## Code Architecture

The BASIC program follows a structured line numbering system:

- **10-200**: Initialization (variables, arrays, first turn setup)
- **2000-2150**: UI rendering (warehouse display, orders display)
- **3000-3060**: Input handler (T=Transport, P=Production, N=Next turn)
- **4000-4100**: Transport order entry
- **4500-4610**: Production order entry  
- **5000-5010**: Order reset functionality
- **6000-6030**: Random province supply generation (5-20 units per commodity)
- **7000-7180**: Core game logic (transport processing, production chains)
- **8000-8070**: Date advancement (7 days per turn, handles month/year rollover)
- **8500-8520**: Month length calculation with leap year support
- **9000-9010**: Header display with current date

## Game Mechanics

The game operates on these key data structures:
- **Warehouse stocks**: TI, LU, FU (timber chain), CO, FB, CL (cotton chain)
- **Transport orders**: TT (timber), CT (cotton)
- **Production orders**: PL (lumber), PF (furniture), PG (fabric), PC (clothes)
- **Province supply**: PT (timber), PCOT (cotton) - regenerated each turn

Production chains:
- 1 Timber → 1 Lumber → 2 Lumber → 1 Furniture
- 1 Cotton → 1 Fabric → 2 Fabric → 1 Clothes

## Key Functions by Line Range

- `iimperialism.bas:38-42`: Main turn processing loop
- `iimperialism.bas:125-129`: Random supply generation
- `iimperialism.bas:131-175`: Core production logic with resource constraints
- `iimperialism.bas:177-187`: Date advancement with proper calendar handling
- `iimperialism.bas:189-191`: Leap year calculation

## Running the Game in microM8 Emulator

To successfully load and run the game in the microM8 Apple II emulator:

1. **Reboot the emulator** using `mcp__microm8__reboot` to ensure clean state
2. **Load the BASIC program directly** using `mcp__microm8__write_interpreter_code`:
   - Set `dialect` to `"fp"` (for Applesoft BASIC)
   - Set `replace` to `true` to overwrite any existing code
   - Include the entire BASIC program content from `iimperialism.bas`
   - **Important**: Add `RUN` at the end of the code to auto-execute
3. **Verify the game is running** by taking a screenshot with `mcp__microm8__screenshot`

**Do NOT** try to:
- Use the microM8 menu system (often gets stuck in feedback dialogs)
- Load from file paths (microM8 uses its own virtual filesystem)
- Use `mcp__microm8__applesoft_write` followed by manual RUN commands

The game will display the warehouse interface with President TACIANO of Haxaco and prompt for T/P/N commands.

## File Structure

- `iimperialism.bas`: Complete game implementation in Applesoft BASIC
- `README.md`: Comprehensive game documentation and mechanics explanation
- `LICENSE`: MIT license