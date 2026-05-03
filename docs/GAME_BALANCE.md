# Game Balance Analysis

## Purpose

This document describes how the current implementation actually plays, which
systems are the main balance levers, and which behaviors are balance-critical
enough that they should be treated as mechanics rather than incidental code
details.

The goal is to keep three phases healthy:

- early game: tight but recoverable
- mid game: real choices between growth, research, warships, and diplomacy
- late game: strong player agency without an automatic snowball

## Current Baseline

The current values in `include/game.h` are:

- `CAPACITY_PER_TRADER_BASE = 2`
- `GUNS_PER_WARSHIP_BASE = 2`
- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT = 50`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT = 50`
- `TRADE_RELATIONS_MULTIPLIER = 2`
- `BATTLE_TRADER_HIT_CHANCE_PERCENT = 5`
- `BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT = 60`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT = 50`
- `DIPLOMATIC_OFFER_COST = 500`
- `RELATIONS_LOSS_PER_TURN = 3`
- `SCIENCE_RESEARCH_COST_MULTIPLIER = 500`
- `UPKEEP_COST_PER_WORKER = 2`
- `UPKEEP_COST_PER_TRADER = 5`
- `UPKEEP_COST_PER_WARSHIP = 5`

The starting state in `src/logic.c` is:

- `$250`
- `4` provinces
- `2` traders
- `2` warships
- `6` idle workers
- `5` idle wagons
- transport orders: `5/5/1/1`
- production orders: `2 lumber`, `2 fabric`, `1 steel`, `1 furniture`,
  `1 clothes`
- `Fair` relations (`100`) with all foreign nations

## What The Current Build Actually Encourages

### 1. The opening treasury is fragile, but the opening industry is not

Opening upkeep is:

- workers: `6 * 2 = 12`
- traders: `2 * 5 = 10`
- warships: `2 * 5 = 10`
- total: `$32` per turn

So the treasury alone lasts about `7` full turns.

That sounds harsher than the real opening because idle workers are the only
workers that pay upkeep. Assigned workers are removed from `available_workers`,
and upkeep is charged only on `available_workers` in `next_turn()`.

That matters because the starting production orders already use `7` workers
while the state also starts with `6` idle workers. In effect, the nation starts
with a larger total labor force than the upkeep math suggests, and can reduce
future labor upkeep by assigning more workers into production or shipbuilding.

This means the opening cash pressure is real, but labor pressure is currently
discounted by implementation.

### 2. The default economy converts stockpiled iron and coal into worker growth

The starting transport and production orders do not form a steady-state loop.

Per turn, before research:

- transport adds `+5 timber`, `+5 wool`, `+1 iron`, `+1 coal`
- production consumes `4 timber`, `4 wool`, `2 iron`, `2 coal`
- net raw change is:
  - timber `+1`
  - wool `+1`
  - iron `-1`
  - coal `-1`
- finished-goods output is:
  - `+1 furniture`
  - `+1 clothes`
  - `+1 steel`

Because furniture and clothes train workers at `1 + 1` each, the opening
economy is set up to turn starting iron and coal reserves into labor growth.
That is a coherent opening pattern, but it also means the initial industrial
setup is more forgiving than the upkeep numbers alone imply.

### 3. Trade margins are reasonable; expedition frequency is the real pressure

At `Fair` relations, `get_relation_tier()` returns tier `2`.

That makes current prices:

- export prices: `85% + 0..15% - 6%` => `79%..94%` of base price
- import prices: `110% + 0..20% + 8%` => `118%..138%` of base price

So trade is not priced to fail. The basic triangle works:

- minor nations export raw materials cheaply
- great powers import raw materials at a premium
- great powers export advanced goods
- minor nations import advanced goods

The real limiter is throughput and voyage risk:

- only `2` traders at start
- only `2` capacity per trader
- only `4` cargo per turn before science
- each expedition has a flat `50%` pirate interception check

The price model supports profit. The player is pressured because each trading
turn has low volume and many chances to get interrupted.

### 4. Relation growth is workable in theory, but brittle in practice

Each unit traded gives:

- `+2` relations from `TRADE_RELATIONS_MULTIPLIER`

Each end turn removes:

- `-3` relations from every non-permanent nation

Going from `Fair` (`100`) to `Great` (`200`) nominally needs `+100`.

If the player spends the full opening capacity of `4` on one nation in a turn,
that is `+8` immediate relation gain, then `-3` at end turn, for `+5` net.
That implies about `20` focused turns to reach `Great` if the player can only
feed one nation with the starting fleet and never misses a turn of trade.

That is already slow. Two additional mechanics make it swingier:

- if treasury reaches `0`, every non-permanent nation is reset immediately to
  `Bad`
- after an offer succeeds, the relation becomes permanent at
  `RELATION_ALLY_COLONY` and never decays

So the system is not a smooth diplomacy grind. It is a long ramp with a harsh
bankruptcy cliff.

### 5. The council is paced around permanent conversions, not goodwill alone

Votes come from provinces:

- player nation: `8`
- each foreign great power: `8`
- each foreign minor nation: `4`
- victory target: `24`

Non-allied major powers vote for themselves. Non-allied minor nations abstain.
Temporary goodwill does not directly convert into council votes.

That means the player must secure permanent partners:

- `2` allied great powers, or
- `1` allied great power plus `2` colonies

This victory structure is sound. The pressure point is still the time needed to
reach `Great` and then survive a `50%` overture roll.

### 6. Sea danger is high per voyage, but fighting is much safer than running

Every trade expedition first checks for pirates:

- `50%` chance

If pirates do not appear, every `Bad` nation rolls separately for interception:

- `50%` chance per `Bad` nation

That creates a nonlinear danger curve. Total expedition battle chance becomes:

- `50%` if no nation is `Bad`
- `75%` if `1` nation is `Bad`
- `87.5%` if `2` nations are `Bad`
- `98.4%` if all `5` nations are `Bad`

That is one of the most important current balance facts. Once relations start
collapsing, trade almost stops being optional risk and starts becoming nearly
guaranteed combat.

Inside combat, the pressure is less punishing than the expedition checks imply:

- early enemy fleets are only `1` ship on turns `1-9`
- trader loss while fighting is only `5%` per enemy response
- trader loss while running is `60%`

So the current system strongly rewards standing and fighting rather than trying
to disengage.

### 7. Naval science is now a direct combat stat, not just flavor

Combat resolution uses firepower:

- player firepower = `visible_friendly_ships * state.guns_per_warship`
- enemy firepower = `visible_enemy_ships * GUNS_PER_WARSHIP_BASE`
- hit chance is weighted by attacker firepower over total firepower

That means:

- `Carronade` doubles player guns per warship from `2` to `4`
- `Shell Guns` raises them again from `4` to `8`
- equal fleets become materially easier fights after gun tech

This makes the science tree much more strategically meaningful than before.
It also means science timing and battle balance can no longer be tuned
independently.

### 8. Research is cheap enough to matter early, but not cheap enough to skip trade

Research costs are linear:

- level 1: `$500`
- level 2: `$1000`
- level 3: `$1500`
- ...
- level 8: `$4000`

Total cost for the full tree is `$18,000`.

That is accessible enough for mid-game planning, but not so cheap that the
player can ignore trade. The bigger issue is branch timing:

- early raw-yield techs strengthen worker growth and production stability
- level 3 gun tech improves naval outcomes before trader-capacity tech arrives
- level 5 doubles trader capacity and also doubles the material cost of newly
  built traders

This is a healthier science model than the older "science is too expensive to
matter" state, but it increases snowball risk for profitable starts.

## Balance-Critical Implementation Details

These are not small edge cases. They currently shape strategy.

### 1. Labor upkeep only charges idle workers

`next_turn()` charges worker upkeep from `state.available_workers`, not from
total workers. Assigning workers into production or shipbuilding reduces labor
upkeep pressure.

If this is intentional, it should be documented as a rule. If it is not
intentional, balance conclusions based on worker cost need to be treated with
caution.

### 2. Relation-based prices lag by one turn

`next_turn()` refreshes market prices first and then decays relations.

That means the relation shown to the player after ending the turn is lower than
the relation tier used to generate that turn's market prices. Better relations
also do not improve prices immediately during the same turn they are earned.

This slightly softens decay in practice, but it is opaque.

### 3. Trader losses reset remaining capacity incorrectly

When a trader is lost in battle, `trader_lost()` sets:

- `state.remaining_turn_capacity = state.traders * CAPACITY_PER_TRADER_BASE`

It does not use `state.capacity_per_trader`, and it does not preserve already
spent capacity for the current turn.

This causes two balance distortions:

- post-science capacity drops to base capacity after a loss
- a mid-turn trader loss can restore cargo capacity that has already been spent

This is significant enough to affect any serious trade-balance conclusions.

### 4. The first diplomatic offer after a long setup is still a pure coin flip

After the player reaches `Great`, the formal alliance or colony attempt is still
just:

- pay `$500`
- succeed on `50%`

Because relation gain is gradual but the conversion is binary, diplomacy can
still feel more swingy than strategic even after the cheaper offer cost.

## Stage-By-Stage Balance Picture

### Early Game: turns 1-9

The current build creates:

- real treasury pressure
- a forgiving industrial base
- profitable but low-volume trade
- frequent voyage interruptions
- a strong incentive to fight rather than run

The opening is not mostly about production starvation. It is mostly about
whether early trade can happen often enough to cover upkeep and start relation
progress.

### Mid Game: turns 10-19

This is where the current balance is most at risk.

By now the player is supposed to:

- have a larger fleet or more traders
- begin buying science
- approach `Great` with at least one nation
- think about council math

But the current numbers still make diplomacy slow, and a single bankruptcy
incident can erase all that progress by sending every non-permanent relation to
`Bad`.

### Late Game: turn 20 onward

Late game depends heavily on whether the player reached one of two states:

- stable trade loop plus permanent diplomatic partners
- early collapse into repeated interceptions and relation failure

Once the player gets permanent allies or colonies, council progress becomes much
more deterministic. The main late-game balance question is whether gun science
and capacity science let a good start snowball too cleanly.

## Highest-Value Balance Levers

### 1. Diplomacy pacing

Most important constants:

- `TRADE_RELATIONS_MULTIPLIER`
- `RELATIONS_LOSS_PER_TURN`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT`

Why:

- these determine whether the player can reach permanent council support before
  the game stalls

Current reading:

- offer cost is no longer the main diplomatic problem
- relation accumulation and final conversion reliability are the problem

### 2. Expedition danger

Most important constants:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT`
- `BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT`

Why:

- expedition frequency is the engine behind both money and diplomacy
- `Bad`-nation interception stacks very aggressively

Current reading:

- the dangerous part is not only pirate chance
- the compounding attack checks after relations sour are the sharper balance
  lever

### 3. Early trade throughput

Most important constants:

- `CAPACITY_PER_TRADER_BASE`
- `UPKEEP_COST_PER_TRADER`
- foreign price base and relation-step percents

Why:

- if trade volume is too low, even good margins do not support diplomacy fast
  enough

Current reading:

- margins are acceptable
- capacity is tight
- upkeep is meaningful but not obviously the first problem

### 4. Science timing

Most important constant:

- `SCIENCE_RESEARCH_COST_MULTIPLIER`

Why:

- research now changes both economic scaling and combat odds

Current reading:

- `500` is already low enough to make science relevant
- lowering it further before other retesting would likely blur the real causes
  of improvement

## Recommended Tuning Order

Do not tune every system at once.

### Pass 1: make diplomacy reachable

Change only:

- `TRADE_RELATIONS_MULTIPLIER`
- or `RELATIONS_LOSS_PER_TURN`
- or `DIPLOMATIC_OVERTURE_CHANCE_PERCENT`

Best first candidates:

- `TRADE_RELATIONS_MULTIPLIER`: `2 -> 3`
- `RELATIONS_LOSS_PER_TURN`: `3 -> 2`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT`: `50 -> 60`

### Pass 2: reduce compounding voyage lockout

Change only:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT`

Best first candidates:

- pirate chance: `50 -> 35`
- foreign interception chance: `50 -> 25` or `30`

The goal is not to remove danger. The goal is to stop bad relations from making
trade almost guaranteed combat.

### Pass 3: re-evaluate science and fleet snowball

Only after diplomacy and expedition flow feel better, re-check:

- `SCIENCE_RESEARCH_COST_MULTIPLIER`
- trader build scaling after capacity tech
- gun-tech combat dominance

## Recommended Non-Constant Fixes Before Deep Rebalance

These are worth resolving before treating simulation results as authoritative:

1. Decide whether worker upkeep should charge total workers or only idle
   workers.
2. Fix `trader_lost()` so remaining capacity uses `state.capacity_per_trader`
   and preserves already-spent cargo for the turn.
3. Decide whether relation-based prices should use pre-decay or post-decay
   status, and document the intended rule.
4. Re-test whether bankruptcy should really collapse every non-permanent nation
   immediately to `Bad`.

## Practical Conclusion

The current build is no longer mainly blocked by science cost or diplomatic
offer cost. Those are both in a playable range.

The main balance problems now are:

- relation growth is still slow relative to council demands
- bankruptcy is too punishing to diplomatic progress
- expedition danger compounds too sharply once relations go bad
- naval science has become powerful enough that it must be tested as a primary
  balance system
- at least one implementation quirk (`trader_lost()`) currently distorts trade
  balance materially

If only one balance area is changed first, it should be diplomacy pacing,
followed immediately by expedition danger. Those are the two systems that most
directly determine whether a campaign becomes strategically tense or merely
swingy.
