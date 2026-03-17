# Project Structure

This repo uses a conventional layout to keep source, headers, assets, tools, and build
artifacts separated.

## Directories

- `src/` - C sources for the resident game code and overlays.
- `include/` - project headers.
- `asm/` - 6502 assembly sources.
- `asm/loader/` - vendored cc65 loader sources used to build `IIMP.SYSTEM`.
- `config/` - cc65 linker configs.
- `assets/` - disk images and other game assets.
- `docs/` - design and technical documentation.
- `tools/` - external build tools (AppleCommander).
- `build/` - build output (objects, binaries, maps, loader).

## Build Outputs

The `Makefile` writes outputs to `build/`:

- `build/iimperialism` main binary
- `build/*.o` object files
- `build/*.bin` overlay binaries
- `build/loader.system` loader system file
- `build/iimperialism.map` linker map

Current overlay binaries include `iscr.bin`, `pscr.bin`, `tscr.bin`, `ascr.bin`,
`dscr.bin`, `texp.bin`, `txac.bin`, `bscr.bin`, `sscr.bin`, and `menu.bin`.

Use `make disk` to update `assets/iimperialism.dsk` with current binaries.
