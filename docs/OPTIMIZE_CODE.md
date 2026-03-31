# Code Size Optimization Notes

This project runs under tight Apple II memory limits, so code size matters as much
as correctness. The goal is to reduce resident or overlay size without changing
behavior.

These guidelines focus on safe, repeatable ways to make code smaller and often
faster at the same time.

## General Principles

- Prefer local optimizations before architectural changes.
- Treat resident memory growth as higher risk than overlay-local changes.
- Measure every change with `make memory-usage`.
- Keep behavior identical unless a gameplay change is explicitly intended.
- If a change saves overlay space by increasing resident size, validate startup on
  disk or emulator, not just the linker output.

## Start With Measurement

Before changing code:

```bash
make memory-usage
```

Useful questions:

- Is the problem in resident memory or in one overlay?
- Which overlay is close to 2KB?
- Is there enough resident headroom to move code safely?
- Did the binary already get close to a known startup failure threshold?

For deeper inspection, generated assembly and object data are useful:

- `cc65 -t apple2 -Oirs -Iinclude -o build/foo.s src/foo.c`
- `ca65 -o build/foo.o -l build/foo.lst build/foo.s`
- `od65 --dump-segments build/foo.o`

## Safe Techniques

These approaches worked well in practice because they stayed inside the overlay and
did not increase resident memory usage.

### Use Narrower Integer Types

cc65 pays a noticeable cost for 16-bit arithmetic.

Prefer `unsigned char` instead of `unsigned int` when:

- the value is known to stay below 256
- the loop count is small
- the index is bounded by a small fixed constant

Typical wins:

- smaller loop code
- fewer helper calls for arithmetic
- less stack traffic

Examples:

- loop variables for lists smaller than 256
- counters bounded by constants like `MAX_VISIBLE_SHIPS`
- helper parameters that only represent small counts

Be careful with:

- values loaded from `GameState` fields stored as `unsigned int`
- arithmetic that may exceed 255
- implicit integer promotion changing the generated code

Note: cc65 with `-Os` already handles most integer promotion cases optimally. Manually
removing `U` suffixes from small constants (`85U` → `85`) has no measurable effect when
`-Os` is active — do not spend time on this.

### Use `register` for Hot Struct Pointer Loops

The `register` keyword places a pointer variable in zero-page, making indirect addressing
cheaper on the 6502. cc65 allows at most two `register` locals per function.

Only apply this when **all** of the following are true:

- the pointer is a pointer-to-struct
- it is dereferenced 3 or more times in the loop body
- the loop executes approximately 100 or more iterations per call

Even when all conditions are met, measure before and after. Savings are usually small
(~50–60 bytes). Do not apply `register` to non-pointer locals or to loops that execute
rarely.

## Replace Arithmetic With Lookup Tables

Division and modulo are expensive on 6502 targets.

If a value comes from a very small bounded domain, precompute it:

- replace `%` with a table lookup
- replace `/` with a table lookup
- replace repeated coordinate math with static offset tables

This is especially effective when:

- the domain is small and fixed
- the table is tiny
- the arithmetic appears inside a hot path or overlay

Tradeoff:

- you spend a few bytes of `RODATA`
- you often save more bytes of `CODE`

This is a good fit for:

- UI coordinates
- sprite positions
- menu navigation offsets
- screen-specific row and column calculations

## Reduce Duplicate Branch Bodies

If two branches do the same work with different constants, load the constants first
and perform the shared call sequence once.

Instead of:

- branch A: set arguments, call helper
- branch B: set different arguments, call helper

Prefer:

- choose constants
- make one helper call

Benefits:

- less duplicated setup code
- fewer repeated call sequences
- better chance for cc65 to emit shorter code

This works well for:

- screen coordinates
- picture IDs
- text positions
- clear/draw helper calls

## Avoid Duplicate Literals

Repeated string literals can waste space, especially inside overlays.

If the same text is used multiple times in one module, prefer one local static
string and reuse it:

- good: `static const char STR_LABEL[] = "Firepower:";`
- avoid repeating the same literal at multiple call sites

This is most useful when:

- the string appears more than once in a constrained module
- the string is only needed locally
- you want to reduce duplicate `RODATA`

For overlays, prefer a local static string over a resident string symbol unless
that resident data is explicitly part of the overlay ABI. Overlays can safely call
resident functions through the jump table, but direct references to resident data
symbols are usually not link-safe. If overlay `RODATA` is the real bottleneck and
resident headroom is available, expose a resident string getter through `JMPTAB`
instead of trying to import raw resident data symbols directly.

## Keep Changes Local First

A local overlay optimization is usually safer than moving code into main memory.

Why:

- resident growth can break startup even if linking succeeds
- jump-table expansion increases coupling
- overlay-local changes are easier to validate

Use local optimization first when:

- you only need a few dozen or a few hundred bytes
- the overlay is near, but not past, the limit
- the code contains obvious 16-bit arithmetic or repeated patterns

Consider moving code resident only when:

- the dependency cluster is clearly isolated
- resident memory has enough verified headroom
- startup has been tested after the change

## Favor Data Over Logic for Fixed Layouts

If the screen layout is fixed, static data is often cheaper than recomputing.

Good candidates:

- fixed X/Y coordinates
- fixed column lists
- precomputed row offsets
- command dispatch tables

Avoid this when:

- the data becomes large enough to outweigh the code savings
- the layout is likely to change often

## Use Direct HGR Writes For Aligned Text

For this project, resident UI text is a special case worth handling separately:

- `print()` always receives character-cell coordinates
- glyphs are 7 pixels wide and 8 pixels tall
- the hot path does not need transparency

That makes text a good fit for direct HGR byte writes instead of `tgi_setpixel()`.

The current implementation uses resident assembly blitters in `asm/text_hgr.s`:

- one HGR byte is written per glyph row
- the blitter scans the string once per row and writes opaque bytes directly
- a 128-byte lookup table reverses the source glyph bit order into Apple II HGR bit order
- a 24-entry row-base table avoids recomputing HGR addresses for `y * 8`

The high-level UI wrappers in `src/ui.c` now live in the Language Card (`LC`)
segment, while the tiny hot-path blitters remain in main-RAM `LOWCODE`. That is a
useful pattern when a helper's call surface must stay resident but its heavier C
logic can be moved out of scarce main-RAM space.

This is a good pattern when all of the following are true:

- coordinates are already aligned to HGR byte boundaries
- overwriting the background is acceptable
- the path is hot enough that TGI call overhead dominates

This is a bad pattern when:

- text must be transparently composited over arbitrary graphics
- callers need arbitrary pixel `x` positions
- exact TGI color semantics matter more than speed

## Minimize Helper Surface Area

Small helper functions are not always cheaper if they force extra argument setup or
stack movement.

Questions to ask:

- does the helper reduce duplicated code enough to justify the call?
- would inlining a trivial helper save stack setup?
- does sharing the helper across multiple call sites still win after compilation?

The right answer depends on generated code, not source elegance.

## Prefer Simpler Control Flow

Complex `switch` blocks, nested conditions, and repeated temporary variables can
expand significantly in cc65 output.

Often smaller:

- early returns
- one shared cleanup path
- fewer temporary locals
- direct bounded loops

Not always smaller:

- clever abstractions
- generalized helpers
- unnecessary parameterization

## Watch Stack Usage

cc65 frequently spills through the C stack. Extra locals and helper arguments can
inflate both code size and runtime overhead.

Ways to reduce this:

- reuse variables where it stays readable
- prefer byte locals when possible
- avoid introducing helpers that take many parameters
- avoid unnecessary temporary pointers

## Verify Every Optimization

After each change:

1. rebuild
2. run `make memory-usage`
3. compare the exact segment or overlay size
4. if resident memory changed, test startup from disk

Do not trust source-level intuition alone. Some changes that look cleaner produce
larger 6502 code.

## Practical Order of Operations

When optimizing a screen or module, use this order:

1. Measure current memory usage.
2. Identify whether the problem is resident or overlay-local.
3. Convert obviously bounded counters and parameters to `unsigned char`.
4. Replace small-domain arithmetic with lookup tables.
5. Merge duplicated branch bodies that only differ by constants.
6. Re-measure.
7. If overlays pass `GameState *s`, consider the parameter-elimination refactor (see above).
8. Only then consider moving code across memory regions.

## Eliminate Parameter Passing to Overlays

Passing a pointer to `GameState` as a parameter to every overlay entry function is
expensive: the caller pushes the pointer, the overlay function receives it, and every
field access is indirect through a register.

A better architecture is to export the current address of `_state` into the overlay linker
config so overlays can reference `state` directly as an `extern`:

1. Find `_state`'s address in `build/iimperialism.map` (it is the first BSS symbol).
2. Generate `build/apple2-ovl.cfg` from `config/apple2-ovl.cfg` with that `_state` export:
   ```
   _state: type = export, value = $809F;
   ```
3. Remove `GameState *s` / `register GameState *s` parameters from all overlay functions.
4. Remove `#define state (*s)` from each overlay file.
5. Update the dispatch in `overlay.c` to call `((void(*)(void))OVERLAY_SLOT)()`.

This was applied to all 11 overlays and saved **~1,017 bytes** of overlay code with no
BSS growth and no change in behavior. It is the highest-value single architectural
change available in this codebase.

**Critical:** overlays must be relinked against a freshly generated
`build/apple2-ovl.cfg` after **any** change that grows or shrinks the resident CODE,
RODATA, DATA, or INIT segments — because those segments precede BSS, and any size
change shifts `_state`'s address. If the generated config is stale, every overlay will
read `GameState` fields from the wrong offset, silently producing garbage values at runtime.

After each build that touches resident code, run:
```
make memory-usage
```
If overlays behave incorrectly after a resident change, compare `_state` in
`build/iimperialism.map` and `build/apple2-ovl.cfg`.

## Compiler Flag Pitfalls

Some flags look promising but do not improve code size for this target.

### `--static-locals` (`-Cl`) — Do Not Use

Adding `-Cl` moves all local variables from the C software stack to BSS/DATA. This
reduces stack-management code in every function, saving overlay and resident code space,
but it grows BSS permanently.

Measured result: −596 bytes of code, **+97 bytes of BSS**. On the Apple II, BSS occupies
RAM just as much as code does. The tradeoff is not acceptable when code size is the
priority.

Do not add `-Cl` to `CFLAGS`.

### `-Oi` (Inlining) — Do Not Use

Adding `-Oi` causes cc65 to inline small functions at each call site. On a desktop
compiler, inlining enables further optimizations. On cc65/6502, it duplicates the function
body at every call site.

Each JSR/RTS pair saved is 6 bytes; each inlined body is much larger. For functions called
from multiple sites, the result is a net increase in code size. Do not add `-Oi` to
`CFLAGS`.

## High-Risk Techniques

Use these only with more validation:

- moving overlay code into resident memory
- expanding the overlay jump table
- introducing new resident helper APIs for overlays
- increasing resident `CODE` / `RODATA` near startup limits

These can save overlay space, but they can also make the main binary stop booting
correctly even when the build succeeds.

One current example is the diplomacy overlay: it uses a resident
`get_diplomacy_string()` helper through `JMPTAB` to keep `dscr.bin` under 2 KB.

## Rule of Thumb

The safest wins usually come from:

- smaller integer types
- lookup tables for tiny fixed domains
- less duplicated argument setup
- keeping optimizations inside the constrained module

These changes preserve functionality, reduce generated code, and avoid shifting
memory pressure into more fragile parts of the program.
