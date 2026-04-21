# Floppy Layout and Autoboot

## Overview

`assets/iimperialism.dsk` is a ProDOS 8 boot disk for the game.

It uses a ProDOS `SYS` loader (`IIMP.SYSTEM`) to auto-start the game binary
(`IIMPERIALISM`) without `BASIC.SYSTEM` or `STARTUP` BASIC.

## Current Disk Contents

Current catalog (from `ac -l`) includes:

- `PRODOS` (`SYS`) - ProDOS 8 operating system (required to boot).
- `IIMP.SYSTEM` (`SYS`) - boot loader executed by ProDOS at startup.
- `IIMPERIALISM` (`BIN`, `A=$0803`) - main game binary.
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

Current size-sensitive entries from the verified build:

- `IIMPERIALISM` uses `68` ProDOS blocks and is `34,166` bytes long.
- Each overlay is still padded to exactly `2,048` bytes and uses `5` ProDOS
  blocks.
- The disk currently has `1,536` bytes free, exactly three ProDOS blocks. Keep at
  least this much free after future changes.

Not present on the game disk:

- `BASIC.SYSTEM`
- `STARTUP`
- `GAME.DATA` (runtime save container; `make disk` removes it from the packaged
  image so the shipped floppy keeps its three-block reserve)

## Autoboot Behavior

Boot sequence:

1. ProDOS boots and runs the first `*.SYSTEM` file (`IIMP.SYSTEM`).
2. `IIMP.SYSTEM` loads `IIMPERIALISM` (BIN) and jumps to its load address.
3. `IIMPERIALISM` initializes game state and later loads screen overlays from disk
   with direct ProDOS MLI reads. The trade expedition market screen is resident
   code; only its action flow is loaded from `TXAC`.
4. The game menu overlay saves/loads `GAME.DATA` with direct ProDOS MLI calls.

## Loader Implementation

Loader sources are vendored in this repo:

- `asm/loader/loader.s`
- `asm/loader/loader.cfg`

The loader is based on `LOADER.SYSTEM` from the cc65 project, originally written by
Oliver Schmidt, and is patched to load the fixed target file `IIMPERIALISM`.

Reference: https://cc65.github.io/doc/apple2.html#s5

Reason: ProDOS filenames are limited to 15 characters, so
`IIMPERIALISM.SYSTEM` is too long. Using short `IIMP.SYSTEM` avoids that limit
while still loading the long game binary name.

## Build and Disk Update

`Makefile` builds and writes the floppy image:

- Builds game binary and overlays.
- Builds loader system file as `build/loader.system`.
- Removes legacy boot files (`STARTUP`, `BASIC.SYSTEM`) from disk image.
- Writes `IIMP.SYSTEM` (`SYS`) and all current game binaries.

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

- `PRODOS`
- `IIMP.SYSTEM`
- `IIMPERIALISM`

The final catalog should report at least:

```text
ProDOS format; 1.536 bytes free
```
