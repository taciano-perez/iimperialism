# Project Structure

This repo uses a conventional layout to keep source, headers, assets, tools, and build
artifacts separated.

## Directories

- `src/` - C sources for the resident game code, final-score helpers, and overlays.
- `include/` - project headers.
- `asm/` - 6502 assembly sources.
- `config/` - cc65 linker configs.
- `assets/` - disk images and other game assets.
- `docs/` - design and technical documentation.
- `tools/` - build and support tools, including AppleCommander and font extraction scripts.
- `third_party/` - vendored upstream sources and ACME binary used by the RWTS boot build.
- `build/` - build output (objects, binaries, maps, RWTS boot staging).

## Build Outputs

The `Makefile` writes outputs to `build/`:

- `build/iimperialism` main binary
- `build/*.o` object files
- `build/*.bin` overlay binaries
- `build/iimperialism.map` linker map
- `build/rwts_boot/*` generated qboot/ProRWTS boot artifacts

The linker map is the authoritative source for segment placement. In the current
build it shows:

- `JMPTAB` in main RAM at `$080F-$0871`
- `LOWCODE` in main RAM at `$0872-$1FFF`
- `LC` in Language Card RAM at `$D400-$DD79`, currently including `src/ui.c`

Current overlay binaries include `iscr.bin`, `pscr.bin`, `tscr.bin`, `ascr.bin`,
`dscr.bin`, `txac.bin`, `bscr.bin`, `sscr.bin`, `menu.bin`, and `cnsl.bin`.

The trade expedition market renderer is resident code in `src/trade_expedition.c`.
The separate `TEXP` overlay was removed so the floppy no longer pays a fixed
5-block file cost for that small screen.

The menu overlay now owns its save/load flow internally, including its ProRWTS
save/load helper and fixed overlay entry stub.

The Council overlay (`cnsl.bin`) owns both the Council of Nations vote table and
the final victory report. Resident code in `src/logic.c` owns the score
calculation and rank selection so the overlay can stay within its 2 KB slot.
The score calculation currently uses only diplomacy, speed, and treasury.

Use `make disk` to update `assets/iimperialism.dsk` with current binaries.
