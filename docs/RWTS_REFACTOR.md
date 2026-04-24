# RWTS / ProDOS Replacement Feasibility

## Short Conclusion

Replacing `PRODOS` with a custom RWTS layer is technically feasible and would
recover the largest amount of floppy space available in the current build, but
it is not a small disk-space optimization. It is a boot-and-storage subsystem
rewrite.

For this project, the best strategy is:

1. keep the disk **ProDOS-formatted**
2. keep the game split into the same named files (`IIMPERIALISM`, `ISCR`,
   `PSCR`, `MENU`, etc.)
3. replace only the **resident ProDOS runtime dependency** with a tiny custom
   boot path plus RWTS-backed file loading
4. defer save/write support until the read-only path is proven

That gives most of the disk-space win without simultaneously redesigning the
file layout, overlay architecture, and save format.

## Current Baseline

The current disk image is tightly coupled to ProDOS:

- boot: ProDOS boots and launches `IIMP.SYSTEM`
- main binary load: `IIMP.SYSTEM` loads `IIMPERIALISM`
- overlays: `asm/prodos_overlay_load.s` uses ProDOS MLI `OPEN` / `READ` /
  `CLOSE`
- save/load: `asm/prodos_gamestate_io.s` uses ProDOS MLI `CREATE` / `OPEN` /
  `READ` / `WRITE` / `CLOSE` / `SET_MARK`

Current catalog from `assets/iimperialism.dsk`:

- `PRODOS` = `34` ProDOS blocks
- `IIMP.SYSTEM` = `1` block
- `IIMPERIALISM` = `68` blocks
- each overlay = `5` blocks
- free space = `3` blocks (`1,536` bytes)

So on paper, removing `PRODOS` alone frees `34` blocks, or `17,408` bytes.
Removing the ProDOS-specific boot file as well would free `35` blocks total.

This is far larger than any current tactical optimization:

- saving one main-binary block is worth `512` bytes
- getting `PSCR` below the next threshold is worth `512` bytes
- removing ProDOS is worth about `17 KB`

## What ProRWTS Changes

`peterferrie/prorwts` is explicitly aimed at ProDOS filesystem access **without
ProDOS resident in memory**. Its README says it can open/read/write binary files
in a ProDOS filesystem, search directories/subdirectories, and occupy only
`5` sectors in memory (`7` with write support). The 6502 Workshop write-up on
ProRWTS also describes it as a small controller that can load files from floppy
or hard disk without ProDOS in memory.

That matters because it means this project would **not** need to abandon the
existing ProDOS file layout in order to remove the `PRODOS` system file from the
disk.

In the most conservative refactor, the disk can remain:

- ProDOS-formatted
- file-oriented
- split into the current main binary plus overlays

The thing that changes is the loader/runtime path:

- custom boot sectors instead of booting the `PRODOS` file
- RWTS-backed file lookup/read instead of ProDOS MLI

## What QBoot Adds

`peterferrie/qboot` is useful to this analysis because it addresses the weakest
part of the current replacement story: bootstrapping.

Its README describes it as a fast bootable track/sector routine that:

- targets multi-loaders in ProDOS order
- loads from any slot
- scatter-reads sectors
- reads at sector level without needing a filesystem API
- requires only `3` pages in memory

That makes `qboot` important evidence that the project would not need to invent
the floppy boot half from nothing.

However, `qboot` does **not** replace the whole ProDOS dependency by itself.
It is best understood as a **boot and raw-load primitive**, not as a complete
substitute for the current file-oriented runtime behavior.

For this project:

- `qboot` is highly relevant for replacing the current ProDOS boot chain
- `prorwts` is highly relevant for replacing the current ProDOS file API usage

So the strongest design is not "choose one or the other," but:

- `qboot`-style bootstrap for the initial floppy load path
- `prorwts` for ongoing file-oriented runtime reads
- write/save support added afterward

## Why This Is Feasible

### 1. Overlay loading is already centralized

All overlay reads go through one resident helper:

- `src/overlay.c`
- `asm/prodos_overlay_load.s`

That is a good seam. Replacing ProDOS MLI with RWTS-backed reads is localized.

### 2. The disk content is already file-based and stable

The game already uses short uppercase filenames and a fixed set of assets:

- main binary
- 10 overlay files
- optional `GAME.DATA`

That fits ProRWTS well. There is no need to invent a packfile format first.

### 3. The save container is compact and fixed-size

`GAME.DATA` is only `949` bytes. Even if file-writing through RWTS proves
awkward, the data size is small enough that a fallback fixed-location save
scheme would be possible.

### 4. The current project already assumes custom low-level disk I/O

The code no longer uses `stdio` for overlays or saves. It already depends on
hand-written assembly helpers for disk access. Conceptually, replacing one
assembly-backed disk layer with another is aligned with the current direction.

## Why This Is Expensive

### 1. Boot must be replaced completely

Today the floppy boots like this:

1. Apple II boot ROM loads boot sector
2. ProDOS boots
3. ProDOS runs `IIMP.SYSTEM`
4. `IIMP.SYSTEM` loads `IIMPERIALISM`

If ProDOS is removed, stages `2-4` must be replaced by a custom chain.

This is the biggest missing piece in the current repository. There is no
existing stage-1/stage-2 floppy bootloader here besides the ProDOS-based
loader.

`qboot` reduces the conceptual risk here because it provides a concrete model
for the boot/read-track-sector half of the problem. It does not remove the need
to integrate and adapt that logic to this project's disk image and build, but it
does mean the boot phase is not purely greenfield.

### 2. Save/load is much more than "read a file"

Overlay loading only needs:

- filename lookup
- sequential full-file read into a fixed address

Save/load needs:

- file create/open
- partial reads
- partial writes
- seek/set-position
- reliable metadata/update behavior

That is a materially larger surface area than the overlay case.

### 3. Build tooling must change

`make disk` currently relies on AppleCommander to update a normal ProDOS disk
image. A custom-boot ProDOS-formatted disk is still possible, but the build will
need extra steps for:

- writing custom boot sectors
- making sure the RWTS bootstrap files land where the boot chain expects them
- validating the image still contains a readable ProDOS filesystem

### 4. Real floppy timing risk goes up

ProDOS MLI hides controller timing details. A custom floppy boot/RWTS path does
not. The 6502 Workshop article specifically calls out timing sensitivity and
drive-to-drive variation in floppy support. That raises validation cost on real
hardware and on multiple emulators.

## Net Disk-Space Outlook

Best-case savings:

- remove `PRODOS`: `34` blocks
- remove `IIMP.SYSTEM`: `1` block

Best-case total recovered space: `35` blocks = `17,920` bytes.

Realistic net savings will be a bit lower because the replacement needs:

- boot sectors
- a small RWTS bootstrap / launcher
- possibly a write-capable RWTS path for saves

Even with that overhead, the likely outcome is still on the order of
`30+` recovered blocks, which dwarfs all other current options.

## Recommended Migration Shape

### Recommended target architecture

Use **qboot-style custom boot + ProRWTS + existing ProDOS filesystem layout**.

Do **not** combine this with:

- overlay repacking
- variable-length overlays
- moving files into a new archive format
- changing overlay IDs or filenames
- changing save-file format unless necessary

The safest version of this project is to preserve the current logical file
model and replace only the storage backend.

### Phase 1: read-only prototype

Goal: prove that a bootable disk can load `IIMPERIALISM` and at least one
overlay without ProDOS resident.

Scope:

- add custom boot sectors
- load a small boot/sector loader, ideally patterned after `qboot`
- load `IIMPERIALISM`
- replace `asm/prodos_overlay_load.s` with an RWTS-backed read helper
- keep the game otherwise unchanged
- temporarily disable or stub save/load if needed

Success criteria:

- the game boots from floppy
- at least one overlay transition works
- ProDOS is no longer required on disk

This phase captures most of the uncertainty while keeping the change set narrow.
It is also the phase where `qboot` is most directly useful.

### Phase 2: full overlay conversion

Once the prototype works:

- switch all overlay loads to RWTS-backed file reads
- remove ProDOS-specific failure reporting/messages
- confirm repeated overlay loads are stable

This should still be manageable because the overlay loader API is centralized.

### Phase 3: save/load replacement

Preferred path:

- keep `GAME.DATA` as a normal file
- use write-capable RWTS support for create/read/write/seek

Why this is preferred:

- it preserves the current save abstraction
- it preserves the current menu logic
- it avoids inventing a second storage model only for saves

Fallback path if write support becomes the blocker:

- preallocate a fixed save area and access it directly with a tiny raw-sector
  helper

That fallback is viable because the save container is only `949` bytes, but it
should be Plan B, not the default design.

## Recommendation Against Doing This First

Although the refactor is feasible, it is **not** the best next optimization if
the immediate goal is simply to recover a little more floppy headroom.

Reasons:

- it changes boot, file I/O, build tooling, and validation all at once
- it introduces real-hardware timing risk
- it is much harder to debug than a main-binary or overlay-size reduction
- the current codebase has no in-tree bootloader groundwork for it

If the goal is "recover one more block soon", this is the wrong project.

If the goal is "buy back ~17 KB and stop fighting ProDOS space forever", this is
the strongest available direction.

## Final Judgment

Feasibility: **yes**

Near-term practicality: **no**

Strategic value: **very high**

Recommended decision:

- treat RWTS replacement as a **separate major refactor**
- do it only if the project is ready to invest in a custom boot path
- when it is done, keep the ProDOS filesystem and current file layout intact
- prototype **qboot-style read-only boot + overlay loading first**
- add save/write support only after that succeeds

That is the highest-confidence route to reclaiming the `PRODOS` space without
turning the disk format, overlay system, and save system into one large
simultaneous rewrite.

## Prototype Lessons

The current RWTS prototype exposed one important implementation lesson that is
worth recording explicitly:

- patching a linked cc65 binary by hand-maintained absolute addresses is too
  brittle for iterative development

The failure mode is straightforward:

- every code-size change in C or assembly can move exported symbols such as
  `callmain`, `zerobss`, `_main`, `_clear_screen`, and `_print_bold`
- some of the required hooks are not just symbol starts, but specific
  instruction boundaries inside crt0/startup
- once one stale address survives a rebuild, the RWTS stage-2 code can corrupt
  startup before the game reaches `main()`

For this reason, the RWTS build should not rely on hardcoded patch constants in
`asm/rwts/continue.s`.

The safer model is:

1. build the RWTS game binary normally
2. parse the live `iimperialism.map`
3. inspect the live startup bytes at `$0803`
4. generate an include file for the continuation stage with the current
   addresses and patch points
5. fail the build if the expected startup pattern no longer matches

That does not remove all risk from binary patching, but it turns the fragile
part from a hand-maintained table into a reproducible build artifact. In
practice, this is the minimum engineering hygiene required for any continued
prototype work that still hooks cc65 startup from the external RWTS bootstrap.

## Prototype Outcome

The read-only prototype now reaches a playable game boot using:

- `qboot` for the raw sector bootstrap
- `prorwts` for opening and loading `IIMP` from the ProDOS filesystem
- a custom RWTS startup handoff instead of the stock ProDOS launch path

Two implementation details turned out to be decisive:

1. the main executable had to be shortened on disk from `IIMPERIALISM` to
   `IIMP`
2. the RWTS bootstrap could not rely on the stock cc65 Apple II `callmain`
   startup path

### What actually failed

The working prototype showed that the disk side was not the final blocker:

- `qboot` successfully loaded the stage-2 bootstrap
- `prorwts_init` returned successfully
- `prorwts_open` successfully loaded the main executable once it was renamed to
  `IIMP`
- the Apple II startup helper and `zerobss` both completed

The remaining failure was after that point, inside cc65 runtime startup. The
linked Apple II startup helper still initialized the cc65 C software stack from
launch-time state that is normally established by the ProDOS boot path. Under
the RWTS bootstrap, that state was not valid.

In practice, the failing assumptions were:

- startup still seeded `c_sp` from `$73/$74`
- the normal `callmain` path still expected the standard ProDOS launch
  environment

### Working bootstrap shape

The current prototype works by doing the following before entering the game:

1. load `IIMP` through ProRWTS
2. patch the startup helper's ProDOS probe to jump directly to the non-ProDOS
   fallback path
3. seed both `$73/$74` and cc65 `c_sp` from `__HIMEM__`
4. let the startup helper and `zerobss` run
5. bypass `callmain`
6. enter `_main` directly from the RWTS continuation

This is not the same as "the stock Apple II cc65 startup now works without
ProDOS." It is a narrower statement:

- the game can boot and run under RWTS with a custom startup handoff
- the failing part of the original path was not ProRWTS file I/O, but the
  ProDOS-dependent launch assumptions embedded in the standard runtime startup

### Design implication

This changes the recommended shape of the refactor slightly.

The earlier recommendation to "keep the game otherwise unchanged" still applies
at the storage/file-layout level, but at startup the project should treat the
RWTS build as having its own entry path.

That means the practical long-term architecture is:

- keep the same ProDOS filesystem layout
- keep the same overlay model
- keep ProRWTS for runtime file loads
- keep qboot for the initial raw bootstrap
- maintain a dedicated RWTS bootstrap handoff into the game instead of trying
  to preserve the exact ProDOS-era `callmain` launch behavior

## References

Local project sources used for this analysis:

- `docs/FLOPPY.md`
- `docs/MEMORY.md`
- `docs/OPTIMIZE_REFACTOR.md`
- `README.md`
- `Makefile`
- `asm/prodos_overlay_load.s`
- `asm/prodos_gamestate_io.s`

External references:

- ProRWTS repository: https://github.com/peterferrie/prorwts
- QBoot repository: https://github.com/peterferrie/qboot
- 6502 Workshop article on ProRWTS and boot/runtime behavior:
  https://www.6502workshop.com/2017/05/tech-talk-prorwts-supercharges-nox.html
