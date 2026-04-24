# Optimization Notes

## Purpose

This file tracks the durable, ongoing disk-space and memory-pressure facts for
the shipped Apple II build. It is not a migration log.

Use it when deciding where size work is still worth doing.

## Current Disk State

From the current shipped build:

- `IIMP` uses `69` ProDOS blocks and is `34,674` bytes long
- each overlay file is still `2,048` bytes and consumes `5` ProDOS blocks
- `GAME.DATA` uses `3` ProDOS blocks
- free space on `assets/iimperialism.dsk` is `17,408` bytes

Important consequence:

- the main binary is still close to a block boundary
- to save one block, `IIMP` needs to drop from `69` total blocks to `68`
- practically, that means shrinking the binary by about `400` bytes is still a
  meaningful target

## Current Memory Pressure

From the latest `make memory-usage` snapshot:

### Resident

| Area | Used | Free |
|------|-----:|-----:|
| `LOWCODE` | 5487 | 543 |
| `RESIDENT_MAIN_SAFE` | 17933 | 499 |
| `LANGUAGE_CARD` | 2635 | 437 |

### Tight overlays

The overlays with the least local slack are currently:

| Overlay | FreeApprox |
|--------|-----------:|
| `txac.bin` | 13 |
| `bscr.bin` | 30 |
| `cnsl.bin` | 44 |
| `dscr.bin` | 70 |
| `tscr.bin` | 88 |
| `ascr.bin` | 7 |

These are the risky ones for future feature additions.

## Practical Priorities

### 1. Save one main-binary block

This remains the highest-value routine size target.

Why:

- a modest resident reduction can still reclaim a real disk block
- it avoids adding new files or changing the shipped disk layout
- it also creates more resident slack for future changes

### 2. Preserve headroom in the tight overlays

Several overlays are close to the 2KB ceiling. Favor local refactors there over
new feature work that grows them further without measurement.

### 3. Variable-length overlays are still a real option

Overlays are still padded to exactly `2048` bytes. That keeps the loader simple,
but it means small overlay shrinkage does not reclaim disk space until an
overlay can move below a ProDOS block threshold.

The strongest candidate remains `PSCR`, because it has the most unused space
inside the 2KB slot. If the overlay storage format is ever changed to stop
padding on disk, `PSCR` is the most likely immediate one-block win.

## Things Already Settled

These are no longer open strategic questions:

- RWTS is the shipped disk/runtime path
- save/load is no longer ProDOS-only
- the auxiliary-memory overlay cache is implemented

That means future optimization work should focus on:

- code size
- overlay slack
- on-disk overlay representation

not on revisiting the old ProDOS-vs-RWTS migration.

## Validation Checklist

After any size-sensitive change:

1. run `make memory-usage`
2. run `java -jar tools/ac.jar -l assets/iimperialism.dsk`
3. check the `IIMP` block count and byte length
4. check the tight overlays, especially `ASCR`, `TXAC`, `BSCR`, `CNSL`, `DSCR`, and `TSCR`
5. verify the game still boots and that overlay transitions still work on both
   64K-style and >64K configurations

