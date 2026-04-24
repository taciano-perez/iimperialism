# RWTS Boot And Runtime

## Overview

The shipped disk boots through qboot and uses resident ProRWTS for:

- loading the main executable `IIMP`
- loading overlay binaries into `OVERLAY_SLOT` at `$8800`
- reading and writing the fixed-size `GAME.DATA` save container

The disk remains ProDOS-formatted, but `PRODOS` and `IIMP.SYSTEM` are no longer
part of the shipped image.

## Boot Chain

The boot path is:

1. qboot runs from raw boot sectors on track 0
2. qboot loads the stage-2 continuation to `$0400`
3. the continuation relocates itself to `$0300`, clears the text screen, and
   initializes ProRWTS
4. the continuation loads `IIMP` to `$0803`
5. the continuation patches the startup handoff and enters the game

The stage-2 source is:

- `asm/rwts/continue.s`

The disk patch/build logic is:

- `tools/build_rwts_boot.py`

The generated boot artifacts go under:

- `build/rwts_boot/`

## Why The Bootstrap Patches Startup

The important practical lesson from the migration is that the storage layer was
not the only thing ProDOS had been providing.

The stock cc65 Apple II startup path still assumed launch-time state that the
traditional ProDOS loader path provided. In the RWTS boot path, the game only
became stable after the continuation did three things before entering the game:

- seeded `$73/$74`
- seeded cc65 `c_sp` from `__HIMEM__`
- bypassed the failing `callmain` path and entered `_main` directly

That is why the RWTS continuation owns a small amount of startup patching. This
is not optional cleanup work; it is part of the working launch environment for
the shipped RWTS build.

## Resident Runtime Layout

After boot:

- ProRWTS is resident at `$BC00-$BFFF`
- the main executable `IIMP` runs from `$0803`
- overlays execute from main RAM at `$8800-$8FFF`

Overlay files remain normal filesystem files:

- `ISCR`, `PSCR`, `TSCR`, `ASCR`, `DSCR`, `TXAC`, `BSCR`, `SSCR`, `MENU`, `CNSL`

The resident disk helpers are:

- `asm/disk_overlay_load.s`
- `asm/disk_gamestate_io.s`

## Overlay Cache On >64K Machines

`init_overlays()` now tries to install cc65's Apple II auxiliary-memory EMD
driver (`a2_auxmem_emd`).

If enough extended-memory pages are available:

- all overlays are preloaded into extended memory once at startup
- later `run_overlay()` calls copy the selected overlay from extended memory to
  `$8800`
- gameplay no longer hits the floppy on each overlay switch

If extended memory is not available:

- the game falls back to loading overlays from disk on demand

This means:

- 64K machines keep the old overlay-loading behavior
- >64K machines trade a longer startup for quieter/faster overlay transitions

## Save Container

`GAME.DATA` is a fixed-size 1024-byte file created by:

- `tools/build_rwts_save.py`

The menu overlay reads and writes it through resident ProRWTS. The current
format is:

- 4-byte container header
- 5 slot records
- each slot = 3-byte save header + `GameState`

Current constants:

- container version = `3`
- save header version = `4`
- slot count = `5`

## Build Dependencies

The RWTS build depends on vendored upstream sources in `third_party/`:

- `third_party/qboot/`
- `third_party/prorwts/`
- `third_party/acme/`

If `third_party/` is missing, rebuild it with:

```powershell
powershell -ExecutionPolicy Bypass -File tools/rebuild_third_party.ps1 -Force
```

That script reclones the upstream sources and rebuilds:

- `third_party/acme/src/acme.exe`

## Related Docs

- `docs/FLOPPY.md`
- `docs/MEMORY.md`
- `docs/STRUCTURE.md`

