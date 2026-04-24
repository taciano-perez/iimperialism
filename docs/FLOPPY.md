# Floppy Layout and Autoboot

## Overview

`assets/iimperialism.dsk` is the shipped boot disk for the game.

It uses a custom qboot + ProRWTS path to auto-start the main game binary
(`IIMP`) without `PRODOS`, `IIMP.SYSTEM`, `BASIC.SYSTEM`, or `STARTUP` BASIC.

## Current Disk Contents

Current catalog (from `ac -l`) includes:

- `IIMP` (`BIN`, `A=$0803`) - main game binary.
- `ISCR` (`BIN`, `A=$8800`) - industry overlay.
- `PSCR` (`BIN`, `A=$9000`) - production overlay.
- `TSCR` (`BIN`, `A=$9800`) - transport overlay.
- `ASCR` (`BIN`, `A=$A000`) - admiralty overlay.
- `DSCR` (`BIN`, `A=$8800`) - diplomacy overlay.
- `TXAC` (`BIN`, `A=$8800`) - diplomacy trade expedition action overlay.
- `BSCR` (`BIN`, `A=$8800`) - battle overlay.
- `SSCR` (`BIN`, `A=$8800`) - science overlay.
- `MENU` (`BIN`, `A=$8800`) - game menu overlay.
- `CNSL` (`BIN`, `A=$8800`) - Council of Nations and final victory report overlay.
- `GAME.DATA` (`BIN`) - fixed-size save container used by the RWTS save/load path.

Current size-sensitive entries from the verified build:

- `IIMP` uses `68` ProDOS blocks and is `34,282` bytes long.
- Each overlay is still padded to exactly `2,048` bytes and uses `5` ProDOS
  blocks.
- The disk currently has `17,920` bytes free. Keep meaningful free space after
  future changes so the fixed save container and boot sectors still fit cleanly.

Not present on the game disk:

- `PRODOS`
- `IIMP.SYSTEM`
- `BASIC.SYSTEM`
- `STARTUP`

## Autoboot Behavior

Boot sequence:

1. qboot runs from the raw boot sectors and loads the stage-2 continuation.
2. The continuation initializes resident ProRWTS and loads `IIMP` to `$0803`.
3. `IIMP` initializes game state and later loads screen overlays from disk with
   resident ProRWTS reads. The trade expedition market screen is resident code;
   only its action flow is loaded from `TXAC`.
4. The game menu overlay saves/loads `GAME.DATA` through resident ProRWTS
   fixed-size read/write calls.

## Boot Implementation

Boot sources live in:

- `asm/rwts/continue.s`
- `tools/build_rwts_boot.py`
- `tools/rebuild_third_party.ps1`
- vendored upstream sources in `third_party/qboot/` and `third_party/prorwts/`

`build_rwts_boot.py` assembles qboot and ProRWTS, generates a live symbol include
from the current linker map, and patches the raw boot sectors plus stage-2
payload into the disk image.

If `third_party/` is missing, `tools/rebuild_third_party.ps1` reclones `acme`,
`qboot`, and `prorwts`, then rebuilds `third_party/acme/src/acme.exe` so the
boot patcher can run again.

## Build and Disk Update

`Makefile` builds and writes the floppy image:

- Builds game binary and overlays.
- Regenerates the fixed `GAME.DATA` image.
- Removes legacy boot files (`PRODOS`, `IIMP.SYSTEM`, `STARTUP`, `BASIC.SYSTEM`)
  from the disk image.
- Writes all current game binaries and patches the qboot/ProRWTS boot sectors.

The final victory report uses the existing `CNSL` overlay and resident helper
functions. It does not add a separate overlay file or any new picture asset.

Primary command:

```bash
make disk
```

On Windows environments where shell redirection differs, use:

```bash
make SHELL=cmd disk
```

## Verification

To inspect disk contents:

```bash
java -jar tools/ac.jar -l assets/iimperialism.dsk
```

Expected boot-critical entries:

- `IIMP`
- `GAME.DATA`

The final catalog should report:

```text
ProDOS format; 17.920 bytes free
```

`PRODOS` and `IIMP.SYSTEM` should be absent from the catalog.
