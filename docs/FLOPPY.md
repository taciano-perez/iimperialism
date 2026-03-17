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
- `TEXP` (`BIN`, `A=$8800`) - diplomacy trade expedition market overlay.
- `TXAC` (`BIN`, `A=$8800`) - diplomacy trade expedition action overlay.
- `BSCR` (`BIN`, `A=$8800`) - battle overlay.
- `SSCR` (`BIN`, `A=$8800`) - science overlay.
- `MENU` (`BIN`, `A=$8800`) - game menu overlay.

Not present on the game disk:

- `BASIC.SYSTEM`
- `STARTUP`

## Autoboot Behavior

Boot sequence:

1. ProDOS boots and runs the first `*.SYSTEM` file (`IIMP.SYSTEM`).
2. `IIMP.SYSTEM` loads `IIMPERIALISM` (BIN) and jumps to its load address.
3. `IIMPERIALISM` initializes game state and later loads screen overlays from disk
   with direct ProDOS MLI reads.

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
