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
7. Only then consider moving code across memory regions.

## High-Risk Techniques

Use these only with more validation:

- moving overlay code into resident memory
- expanding the overlay jump table
- introducing new resident helper APIs for overlays
- increasing resident `CODE` / `RODATA` near startup limits

These can save overlay space, but they can also make the main binary stop booting
correctly even when the build succeeds.

## Rule of Thumb

The safest wins usually come from:

- smaller integer types
- lookup tables for tiny fixed domains
- less duplicated argument setup
- keeping optimizations inside the constrained module

These changes preserve functionality, reduce generated code, and avoid shifting
memory pressure into more fragile parts of the program.
