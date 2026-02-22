# Project Structure

This repo uses a conventional layout to keep source, headers, assets, tools, and build
artifacts separated.

## Directories

- `src/` - C sources for the main binary and overlays.
- `include/` - Project headers.
- `asm/` - 6502 assembly sources.
- `config/` - cc65 linker configs.
- `assets/` - Disk image, BASIC launcher, and image assets.
- `docs/` - Design and technical documentation.
- `tools/` - External build tools (AppleCommander).
- `build/` - Build output (objects, binaries, maps, overlays).

## Build Outputs

The `Makefile` writes all outputs to `build/`:
- `build/iimperialism` main binary
- `build/*.o` object files
- `build/*.bin` overlay binaries
- `build/iimperialism.map` linker map

Use `make disk` to update `assets/iimperialism.dsk` with the latest binaries.
