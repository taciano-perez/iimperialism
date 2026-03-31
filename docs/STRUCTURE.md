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
- `tools/` - build and support tools, including AppleCommander and font extraction scripts.
- `build/` - build output (objects, binaries, maps, loader).

## Build Outputs

The `Makefile` writes outputs to `build/`:

- `build/iimperialism` main binary
- `build/*.o` object files
- `build/*.bin` overlay binaries
- `build/loader.system` loader system file
- `build/iimperialism.map` linker map

The linker map is the authoritative source for segment placement. In the current
build it shows:

- `JMPTAB` in main RAM at `$080F-$0868`
- `LOWCODE` in main RAM at `$0869-$1631`
- `LC` in Language Card RAM at `$D400-$DD79`, currently including `src/ui.c`

Current overlay binaries include `iscr.bin`, `pscr.bin`, `tscr.bin`, `ascr.bin`,
`dscr.bin`, `texp.bin`, `txac.bin`, `bscr.bin`, `sscr.bin`, `menu.bin`, and
`cnsl.bin`.

The menu overlay now owns its save/load flow internally, including its ProDOS
MLI helper and fixed overlay entry stub.

Use `make disk` to update `assets/iimperialism.dsk` with current binaries.
