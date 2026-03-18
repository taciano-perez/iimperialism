# Overlay Merge Refactor Analysis

## Goal

Evaluate whether two small overlays can be packed into one 2 KB overlay image and
still be invoked from resident code.

Current constraint:

- `run_overlay()` loads one file and always jumps to `$8800`.
- This means a merged overlay needs a dispatcher at `$8800`.
- Different `run_overlay()` calls can still exist, but they cannot jump to
  different entry offsets directly under the current ABI.

Relevant files:

- `src/overlay.c`
- `include/overlay.h`
- `config/apple2-ovl.cfg`
- `docs/MEMORY.md`

## Current Overlay Sizes

From `make memory-usage` on 2026-03-18:

| Overlay | UsedApprox | UsedPercent |
|---------|------------|-------------|
| `iscr.bin` | 783 | 38.23% |
| `pscr.bin` | 988 | 48.24% |
| `tscr.bin` | 842 | 41.11% |
| `menu.bin` | 915 | 44.68% |
| `texp.bin` | 809 | 39.50% |
| `dscr.bin` | 1190 | 58.11% |
| `bscr.bin` | 1425 | 69.58% |
| `sscr.bin` | 1553 | 75.83% |
| `ascr.bin` | 1695 | 82.76% |
| `txac.bin` | 1939 | 94.68% |

Notes:

- `UsedApprox` is not linker-exact. It is derived from the last non-zero byte in
  the emitted `.bin`.
- A merged candidate needs to fit within 2048 bytes after adding a dispatcher
  stub and any merged runtime/linker overhead.

## Ranking

### 1. `industry + transport`

Raw combined size: `783 + 842 = 1625`

Why this ranks first:

- Clear functional relationship: both belong to the same industry/warehouse loop.
- Easy dispatch key: `state.current_screen` already distinguishes the two.
- Good headroom: about 423 bytes before dispatcher overhead.
- Neither overlay pulls in special helper objects like menu save/load code.

Risk:

- Moderate long-term growth risk if either screen gains more text or logic.

### 2. `industry + production`

Raw combined size: `783 + 988 = 1771`

Why it is strong:

- Same ministry flow and shared presentation style.
- Easy dispatch from `state.current_screen`.
- Still leaves useful headroom: about 277 bytes.

Why it ranks below `industry + transport`:

- Less spare room.
- `production` has more text and interaction detail, so future growth pressure is
  higher.

### 3. `transport + production`

Raw combined size: `842 + 988 = 1830`

Why it is still viable:

- Same screen family.
- Easy dispatch from `state.current_screen`.

Why it ranks below the first two:

- Only about 218 bytes of headroom before stub overhead and future feature growth.
- This is probably still workable today, but less comfortable as a maintenance
  target.

### 4. `diplomacy + trade market`

Raw combined size: `1190 + 809 = 1999`

Why it is interesting:

- Strong flow relationship: diplomacy leads directly into trade expedition setup.
- Dispatch can still be derived from `state.current_screen` because diplomacy and
  trade expedition use different screen IDs.

Why it ranks lower:

- Only about 49 bytes of nominal headroom before adding a dispatcher.
- `UsedApprox` may be optimistic for this purpose.
- This is close enough to the limit that one small text or logic change could
  break the merge.

Conclusion:

- Feasible only as a tight fit experiment, not as a robust refactor target.

### 5. `industry + trade market`

Raw combined size: `783 + 809 = 1592`

Why it fits:

- Comfortable size headroom.

Why it ranks low despite that:

- Weak functional coupling.
- Dispatch would require sharing an overlay between unrelated flows.
- The result would save disk space, but would make the codebase less obvious.

Conclusion:

- Technically feasible, architecturally poor.

### 6. `transport + trade market`

Raw combined size: `842 + 809 = 1651`

Assessment:

- Fits by size.
- Same basic issues as `industry + trade market`.
- Low semantic cohesion makes future maintenance worse than the disk-space win is
  worth.

### 7. `production + trade market`

Raw combined size: `988 + 809 = 1797`

Assessment:

- Also fits by size.
- Same poor cohesion as the other cross-flow pairings.
- Less headroom than the `industry`/`transport` cross-pair variants.

### 8. Any pairing involving `menu`

Examples that fit by raw size:

- `menu + industry` = 1698
- `menu + transport` = 1757
- `menu + production` = 1903
- `menu + trade market` = 1724

Why these rank low:

- `menu` is invoked from `ESC`, not from a unique `state.current_screen`.
- Dispatch cannot be inferred reliably from current screen alone.
- The menu overlay also links extra save/load helper code, so future growth is
  more likely.

Conclusion:

- Feasible only if resident code adds an explicit selector or request variable.
- Not a good first merge target.

### 9. `trade market + trade action`

Raw combined size: `809 + 1939 = 2748`

Why it ranks last:

- Semantically this is the most natural pair.
- Numerically it is not even close to fitting.
- It would also need explicit entry selection because both are used within the
  trade-expedition flow, not distinguished only by `state.current_screen`.

Conclusion:

- Not feasible under the current 2 KB overlay budget.

## Summary Recommendation

Best candidates:

1. `industry + transport`
2. `industry + production`
3. `transport + production`

Possible but fragile:

1. `diplomacy + trade market`

Avoid for the first pass:

1. Any pairing involving `menu`
2. Cross-flow pairings with weak cohesion
3. `trade market + trade action`

## Suggested Refactor Strategy

If this is pursued later, the safest order is:

1. Merge one pair from the `industry` / `transport` / `production` family.
2. Keep separate overlay IDs if useful, but map them to the same file.
3. Add one assembly entry stub at `$8800` that dispatches based on
   `state.current_screen`.
4. Re-run `make memory-usage` after linking the merged image to confirm the real
   post-link size.

## Important Non-Goal

This refactor does not reduce runtime RAM usage.

It only reduces disk footprint and overlay file count. The execution window at
`$8800-$8FFF` remains 2 KB either way.
