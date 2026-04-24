# Overlay Disk Refactor: Aux-Memory Overlay Cache Feasibility

## Scope

Question analyzed:

- can the game detect whether the machine has only `64K` or has auxiliary / extended
  memory (`128K` Apple IIe/IIc class machines)?
- if auxiliary memory is present, can overlays be copied there and reused from RAM
  instead of rereading the floppy every time?
- can this be done while preserving the current shipped floppy reserve of
  `1,536` bytes?

This note is analysis only. No runtime code changes were made.

## Current Baseline

From the current tree and build outputs on `2026-04-21`:

- there are `10` overlay files
- each overlay is still exactly `2,048` bytes on disk
- total overlay payload is therefore `20,480` bytes
- current main binary size is `34,296` bytes
- current disk free space is `1,536` bytes

Current build measurements:

- `IIMPERIALISM` is `68` ProDOS blocks
- overlays are all `5` blocks each
- `RESIDENT_MAIN_SAFE` free space in RAM is `1,104` bytes
- `LOWCODE` free space in RAM is `724` bytes

Important disk fact:

- `34,296` bytes is only `8` bytes below the next `512`-byte data-block boundary
  (`67 * 512 = 34,304`)
- so if the main binary grows by more than `8` bytes, `IIMPERIALISM` will go from
  `68` blocks to `69` blocks
- that would cut the floppy reserve from `1,536` bytes to `1,024` bytes

That one number dominates the practical conclusion.

## Hardware / ProDOS Facts

### 1. Detecting `64K` vs `128K` is feasible

Under ProDOS, this is straightforward.

- ProDOS exposes the machine configuration in `MACHID` at `$BF98`
- bits `5:4` encode memory size
- `11` means `128K`

That means the game does not need a large custom probe just to decide whether to
use an overlay cache.

If we ever wanted a standalone non-ProDOS-safe detector, Apple also published a
larger Apple II family identification routine that distinguishes `64K` vs `128K`
on IIe systems by testing auxiliary-memory behavior. That is available, but it is
larger than needed here.

### 2. Auxiliary memory is large enough

The overlay set is only:

- `10 * 2,048 = 20,480` bytes

The auxiliary bank on a normal `128K` IIe/IIc machine is `64K`, so raw capacity is
not the problem.

### 3. The cache should not execute directly from auxiliary RAM

The safe design is:

1. keep executing overlays from the existing main-RAM slot at `$8800-$8FFF`
2. use auxiliary RAM only as a backing store / cache
3. copy `2KB` from aux RAM to `$8800` before each execution

Reasons:

- ProDOS documents that the alternate `64K` bank cannot contain code that makes
  MLI calls and cannot be used for system buffers
- Apple warns against calling firmware with auxiliary read/write mapping still
  enabled
- the current overlay ABI assumes normal main-memory execution at `$8800`

So the right optimization is "cache in aux, execute in main", not "execute in aux".

### 4. `/RAM` must be handled properly

This is the subtle part.

On `128K` ProDOS machines, auxiliary memory is typically exposed as `/RAM`.
Apple's ProDOS notes say that if `/RAM` is enabled, applications may use
auxiliary memory above `$0800` only after removing `/RAM`, and should reinstall it
when done.

So a correct implementation should assume:

- if the machine is `64K`: no aux cache
- if the machine is `128K`: disconnect `/RAM`, use aux cache, reinstall `/RAM`
  on exit/reset if we want to be polite

This is doable, but it is not free in code size.

## Implementation Shape

## Recommended memory layout

If the cache is enabled, the cleanest fixed layout is:

- overlay `0` at aux `$0800-$0FFF`
- overlay `1` at aux `$1000-$17FF`
- overlay `2` at aux `$1800-$1FFF`
- ...
- overlay `9` at aux `$5000-$57FF`

This uses exactly `20,480` bytes and keeps the mapping simple:

- slot base = `$0800 + id * $0800`

That is compact to compute because overlay IDs are already `0..9`.

## Required code pieces

A practical implementation needs:

1. `init_overlays()`
   - read `MACHID`
   - decide whether aux cache is enabled
   - on `128K`, disconnect `/RAM` if using aux memory properly
2. cache metadata
   - at minimum: one `10`-bit or `10`-byte valid table
3. one or two assembly copy routines
   - main -> aux, `2KB`
   - aux -> main, `2KB`
4. `run_overlay(id)` changes
   - if cached: aux -> `$8800`
   - else: load from disk into `$8800`, then copy `$8800` -> aux, then mark valid

## Assembly is the right place for the copy path

This part should be handwritten assembly, not C.

Reasons:

- it needs controlled use of soft switches
- it must restore normal memory mapping before returning
- it needs to stay very small
- it runs in a hot path

A single page-counted copy loop for `8` pages is enough. The code should:

- execute from main RAM
- switch only read/write mapping as needed
- avoid firmware or MLI calls while aux mapping is active
- restore `RAMRD` and `RAMWRT` to main RAM before exit

## Startup preload vs lazy cache

There are two variants.

### A. Preload all overlays at startup

Behavior:

- on `128K`, read all `10` overlays once during boot
- copy each to its aux slot
- later overlay switches do only aux -> main copies

Pros:

- zero floppy access during gameplay after startup
- simplest runtime cache state

Cons:

- long noisy startup
- all disk cost is paid up front even if some overlays are never used
- likely a little more setup code

### B. Lazy cache on first use

Behavior:

- on first visit to an overlay: load from disk, copy to aux
- on later visits: aux -> main only

Pros:

- lower startup cost
- same end-state benefit for repeated screens
- probably the smallest implementation

Cons:

- first visit to each overlay still hits the floppy once

Given the current size pressure, lazy caching is the better first implementation
even though the original idea was startup preload.

## Main Constraint: Disk Footprint

Architecturally, this feature is feasible.

As a shipped change right now, under the requirement:

- keep floppy free space at `1.5K` as it is today

it is **not realistically feasible yet**.

Reason:

- the current main binary has only `8` bytes of block-boundary headroom
- a real aux-cache implementation needs more than `8` bytes
- so even a careful implementation will almost certainly push `IIMPERIALISM`
  from `68` to `69` ProDOS blocks
- that immediately reduces free space below the required `1,536` bytes

This is true even if the runtime RAM layout still has room. RAM headroom is not
the blocker; disk block accounting is.

## Estimated code-cost direction

I did not code this yet, but the likely cost profile is:

- `MACHID` check: tiny
- cache-valid tracking: tiny
- aux copy routine(s): modest, but not tiny
- `/RAM` disconnect/reinstall logic: nontrivial
- overlay dispatch changes: modest

Even an optimized assembly-heavy version is very unlikely to fit inside the
current `8`-byte disk headroom.

## Practical Recommendation

### Conclusion

The feature is feasible in principle, but **not as an immediate drop-in change**
if the floppy must stay at the current `1.5K` reserve.

### Best sequence

1. Recover at least `1` real ProDOS block elsewhere first.
2. Then implement an aux-memory overlay cache.
3. Prefer lazy caching first.
4. Only switch to eager startup preload if the user experience gain is worth the
   extra startup delay.

### Best precursor candidates

The cleanest ways to create room first are:

- recover a main-binary block by shrinking `IIMPERIALISM`
- or recover an overlay block with variable-length overlays, most likely from
  `PSCR`

Right now `PSCR` is the strongest overlay candidate because it is already around
`1,512` bytes used and is close to a `1,536`-byte threshold.

If one disk block is recovered first, then this aux-cache feature becomes much
more realistic.

## Recommended Design If/When Implemented

If this moves forward later, the recommended design is:

- keep the current disk overlay files unchanged
- keep executing overlays from `$8800`
- use auxiliary RAM only as a cache
- detect `128K` via `MACHID`
- disconnect `/RAM` on cache-enabled systems
- use a tiny assembly copy routine
- start with lazy caching

That gives:

- full compatibility with `64K` machines: still read overlay from disk every time
- much quieter and faster repeat overlay transitions on `128K` machines
- no need to redesign the overlay ABI

## Bottom Line

Bottom-line answer:

- `64K` vs `128K` detection: yes, easy
- caching overlays in auxiliary memory: yes, technically straightforward
- executing overlays directly from auxiliary memory: no, not the right design
- shipping it now while preserving the exact current floppy reserve: no, not
  without first freeing at least one block elsewhere

## References

Repo-local:

- [src/overlay.c](src/overlay.c)
- [asm/disk_overlay_load.s](asm/disk_overlay_load.s)
- [include/overlay.h](include/overlay.h)
- [docs/MEMORY.md](docs/MEMORY.md)
- [docs/FLOPPY.md](docs/FLOPPY.md)
- [docs/OPTIMIZE_REFACTOR.md](docs/OPTIMIZE_REFACTOR.md)

External Apple / ProDOS references used for this analysis:

- ProDOS 8 Technical Reference, "Writing a ProDOS System Program"  
  https://prodos8.com/docs/techref/writing-a-prodos-system-program/
- ProDOS 8 Technical Note #21, device identification and auxiliary-slot RAM disk notes  
  https://prodos8.com/docs/technote/21/
- ProDOS 8 Technical Note #26, "Polite Use of Auxiliary Memory"  
  https://mirrors.apple2.org.za/apple.cabi.net/FAQs.and.INFO/A2.TECH.NOTES.ETC/A2.CLASSIC.TNTS/p8026.htm
- Apple II Miscellaneous Technical Note #2, family identification routines  
  https://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Computers/Apple%20II/Apple%20II/Documentation/Misc%20%23002%20Family%20ID%20Routines.pdf
