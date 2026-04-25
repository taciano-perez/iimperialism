# Game Balance Analysis

## Purpose

This document summarizes how the current rules are likely to play, which
constants in `include/game.h` are the strongest balance levers, and how to
model simulated playthroughs before changing gameplay code.

The goal is not just "make the game harder" or "make it easier". The goal is:

- early game: the player should feel constrained, but not doomed
- mid game: the player should be making meaningful tradeoffs between economy,
  science, navy, and diplomacy
- late game: the player should still face pressure, but should have enough
  agency to close out a winning diplomatic campaign

## What The Current Rule Set Encourages

### 1. The opening economy is fragile

The player starts with:

- `$250`
- `6` available workers
- `2` traders
- `2` warships

With the current upkeep constants:

- `UPKEEP_COST_PER_WORKER = 2`
- `UPKEEP_COST_PER_TRADER = 5`
- `UPKEEP_COST_PER_WARSHIP = 5`

the opening upkeep is:

- `6 * 2 + 2 * 5 + 2 * 5 = $32 per turn`

That means the starting treasury lasts about `7-8` turns without successful
trade. This is a good signal that the player must engage with trade quickly, but
it also means the opening is very punishing if battles, unlucky prices, or poor
first decisions interrupt early profits.

### 2. Diplomatic progression is currently slow relative to council timing

A nation starts at `RELATION_FAIR = 100`. To reach `RELATION_GREAT = 200`,
the player needs `+100` relation.

Current relation movement:

- gain from trade: `quantity * TRADE_RELATIONS_MULTIPLIER`
- current multiplier: `2`
- end-turn decay: `RELATIONS_LOSS_PER_TURN = 3`

At the start of the game, the player has only `4` trade capacity per turn
(`2` traders * `2` capacity). If all `4` capacity is used on one nation, the
gross gain is `+8`, and the end-turn decay makes the net gain only `+5`.

That implies roughly `20` focused turns to move a nation from `Fair` to
`Great`, before even attempting an alliance or colony offer.

Since the council meets every `10` turns and victory needs at least:

- `1` allied great power and `2` colonies, or
- `2` allied great powers

the current diplomatic pacing is likely too slow for a satisfying mid game.
The player will often feel that the game is asking for diplomacy, while the
numbers mostly permit only a long grind.

### 3. Trade is the main engine, but early trade throughput is small

Current early trade throughput is constrained by:

- `CAPACITY_PER_TRADER_BASE = 2`
- only `2` starting traders
- `50%` pirate battle chance per expedition
- extra foreign attack checks from bad-relation nations

The price system itself is directionally sound:

- great powers import raw materials and export finished goods
- minor nations export raw materials and import finished goods
- better relations improve both buy and sell terms

This creates a useful trade triangle. The issue is not the concept. The issue is
that early expedition volume is low while expedition risk is high, so the player
can be forced into a thin-margin economy.

### 4. Science is powerful, and now arrives much earlier

Current research cost is:

- `level * SCIENCE_RESEARCH_COST_MULTIPLIER`
- multiplier is `500`

So the ladder costs:

- `500, 1000, 1500, ... 4000`

The upgrades are strong, especially:

- raw yield doublings
- trader capacity doublings
- warship gun multipliers

This is no longer a late-game-only system. A competent player can plausibly buy
early research in the opening or early mid game. That improves the game's tempo,
but it also increases snowball risk: if a player gets even a modestly profitable
trade start, science can now accelerate the economy much sooner than before.

### 5. Battles create pressure, but the pressure is front-loaded

Current expedition danger:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT = 50`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT = 50`

This means early trade, which the player urgently needs, is already exposed to a
coin-flip pirate encounter. If any nation has fallen to `Bad`, that adds more
attack pressure on top.

That kind of pressure is useful in the late game, but it is probably too high
for the opening and too binary for the middle.

### 6. Formal diplomacy is cheaper, but relation growth is still the bottleneck

Current formal diplomacy values:

- `DIPLOMATIC_OFFER_COST = 500`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT = 50`

Halving the offer cost is a strong improvement. It means the player is no longer
punished as severely for converting a `Great` relation into an ally or colony.
This reduces one of the game's sharpest mid-game treasury taxes.

However, the player still needs to reach `Great` first, and that pacing is still
controlled mostly by:

- `TRADE_RELATIONS_MULTIPLIER = 2`
- `RELATIONS_LOSS_PER_TURN = 3`
- low starting trade capacity
- high trade-expedition battle pressure

So diplomacy is less expensive now, but not yet necessarily faster.

### 7. Naval gun science now directly affects combat outcomes

The battle model has been refactored so hit odds are now weighted by firepower,
not just by ship count.

Current battle model:

- player firepower = `visible_friendly_ships * state.guns_per_warship`
- enemy firepower = `visible_enemy_ships * GUNS_PER_WARSHIP_BASE`
- chance to sink a ship = `attacker_firepower / (attacker_firepower + defender_firepower)`

This is a meaningful change for balance:

- `Carronade` and `Shell Guns` are now real combat upgrades
- a smaller player fleet with better guns can now fight evenly or even
  advantageously against a larger enemy fleet
- naval science is now a stronger strategic branch than before

This improves coherence, because the displayed `Firepower:` value now matches the
actual battle odds. It also increases the risk that gun science becomes one of
the strongest snowball mechanics in the game, especially now that research is
cheaper.

### 8. Several balance outcomes are currently dominated by implementation details

One current behavior still matters more than any constant tuning:

1. Upkeep is charged only on `available_workers`, not on the total labor force.
   Assigning workers into production reduces labor upkeep pressure.

If this is not intentional, then purely tuning `game.h` constants will not fully
solve balance.

## Stage-By-Stage Balance Targets

These are useful target outcomes for tuning.

### Early Game: turns 1-8

Desired feeling:

- cash is tight
- mistakes matter
- the player can still recover from one unlucky battle or one bad trade choice

Suggested target outcomes:

- bankruptcy should be uncommon before turn `8`
- first meaningful profitable trade loop should be possible by turns `2-4`
- first research purchase should feel reachable by turns `3-6`
- the player should not lose merchant capacity too often before they have room to
  rebuild

### Mid Game: turns 9-20

Desired feeling:

- the player starts specializing
- diplomacy becomes a deliberate strategy, not a background grind
- the first council should often show progress, the second should feel
  winnable for strong play

Suggested target outcomes:

- first `Great` relation should usually happen around turns `8-12`
- first alliance/colony should usually happen around turns `9-14`
- the player should be choosing between:
  - more traders
  - more warships
  - science investment
  - training workers
- the second council should often be competitive, not merely informational

### Late Game: turns 21+

Desired feeling:

- the empire is larger and more efficient
- upkeep and battle losses still matter
- victory should require closing decisions, not just waiting for an inevitable win
- gun science should feel strong, but should not make a small elite fleet too
  dominant for the rest of the campaign

Suggested target outcomes:

- winning by the third council should be realistic for strong play
- winning by the fourth council should still be possible for weaker or unlucky runs
- trade should remain relevant even after key science upgrades
- diplomacy should still require maintenance unless the player has already locked
  nations into ally/colony status

## The Most Important `game.h` Levers

### 1. Diplomacy pacing

Highest-value constants:

- `TRADE_RELATIONS_MULTIPLIER`
- `RELATIONS_LOSS_PER_TURN`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT`
- `DIPLOMATIC_OFFER_COST`

Recommended direction:

- increase `TRADE_RELATIONS_MULTIPLIER`
- reduce `RELATIONS_LOSS_PER_TURN`
- make the final ally/colony conversion somewhat more reliable

Why:

- this is the main bottleneck for reaching the council victory condition
- it directly determines whether the game peaks in the mid game or stalls there

Practical first-pass tuning candidates:

- `TRADE_RELATIONS_MULTIPLIER`: try `3` first, then `4` if still too slow
- `RELATIONS_LOSS_PER_TURN`: try `2` first
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT`: try `60-70`
- `DIPLOMATIC_OFFER_COST`: current `500` is already in a reasonable range

Interpretation:

- raising trade relation gain rewards active play
- lowering decay reduces the feeling that diplomacy is a treadmill
- raising offer success reduces frustration from coin-flip failures after a long
  setup
- offer cost is no longer the main diplomatic problem

### 2. Early trade viability

Highest-value constants:

- `CAPACITY_PER_TRADER_BASE`
- `UPKEEP_COST_PER_TRADER`
- `FOREIGN_EXPORT_PRICE_BASE_PERCENT`
- `FOREIGN_EXPORT_PRICE_VARIANCE_PERCENT`
- `FOREIGN_IMPORT_PRICE_BASE_PERCENT`
- `FOREIGN_IMPORT_PRICE_VARIANCE_PERCENT`
- `FOREIGN_EXPORT_PRICE_RELATION_STEP_PERCENT`
- `FOREIGN_IMPORT_PRICE_RELATION_STEP_PERCENT`

Recommended direction:

- either increase early throughput or reduce early fixed fleet pressure
- preserve the relation-based price advantage because it is one of the game's
  best systems
- since science is now cheaper, be careful about over-buffing both early trade
  and research at the same time

Practical first-pass tuning candidates:

- `CAPACITY_PER_TRADER_BASE`: test `3`
- `UPKEEP_COST_PER_TRADER`: test `4`
- `FOREIGN_EXPORT_PRICE_BASE_PERCENT`: keep near `80-85`
- `FOREIGN_IMPORT_PRICE_BASE_PERCENT`: keep near `115-120`
- relation step percents can be nudged slightly upward if diplomacy still does
  not feel economically meaningful

Interpretation:

- if trade margins are acceptable but total volume is too small, raise capacity
- if volume is fine but treasury still collapses, reduce upkeep
- if both are weak, modestly widen buy/sell spreads

### 3. Battle pressure

Highest-value constants:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT`
- `BATTLE_TRADER_HIT_CHANCE_PERCENT`
- `BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT`
- `BATTLE_BOUNTY_VARIANCE_PERCENT`

Recommended direction:

- reduce early forced battles
- keep the "bad relations are dangerous" idea, but avoid making trade impossible
  once one nation collapses to `Bad`

Practical first-pass tuning candidates:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT`: test `30-35`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT`: test `25-35`
- `BATTLE_RUN_TRADER_HIT_CHANCE_PERCENT`: test `35-45`
- `BATTLE_BOUNTY_VARIANCE_PERCENT`: consider `25-35` for less swing

Interpretation:

- players should fear the sea, but not feel that trade is a coin-flip tax
- late-game danger can come from larger fleets and accumulated geopolitical
  hostility rather than constant early interception
- battle pressure is now a more important pacing lever because cheaper science
  and cheaper overtures have reduced two other forms of economic friction
- because gun science now affects hit odds directly, battle balance must be
  evaluated together with science timing, not in isolation

### 4. Research timing

Highest-value constants:

- `SCIENCE_RESEARCH_COST_MULTIPLIER`

Recommended direction:

- the multiplier is no longer obviously too high
- avoid lowering it further until diplomacy and trade pacing are re-tested

Practical first-pass tuning candidates:

- current `500` may already be close to the floor for a healthy campaign pace
- if science snowball becomes dominant, consider moving back upward slightly
  rather than lowering it further

Interpretation:

- this is a coarse lever because it affects the whole tree equally
- if early science feels good but late science arrives too quickly, the eventual
  solution may need per-tech costs rather than one global multiplier
- this matters even more now because the gun-science branch directly improves
  battle odds instead of only improving displayed firepower and build costs

### 5. Fixed upkeep pressure

Highest-value constants:

- `UPKEEP_COST_PER_WORKER`
- `UPKEEP_COST_PER_TRADER`
- `UPKEEP_COST_PER_WARSHIP`

Recommended direction:

- treat these carefully until the labor-upkeep behavior is confirmed as
  intentional

Practical first-pass tuning candidates:

- `UPKEEP_COST_PER_WARSHIP`: test `4`
- `UPKEEP_COST_PER_TRADER`: test `4`
- leave `UPKEEP_COST_PER_WORKER` alone until the worker-upkeep rule is settled

Interpretation:

- warship and trader upkeep shape whether the player can afford expansion
- worker upkeep is currently distorted by how assigned workers are stored

## Constants That Matter Less For Core Balance

These are secondary levers, not primary ones:

- `COUNCIL_VICTORY_VOTES`
- score-related constants
- `FOREIGN_NATION_COUNT`
- `FOREIGN_TRADE_ENTRY_COUNT`

The council target of `24` out of `32` is structurally good. It creates a clear
need for meaningful diplomacy. The bigger issue is not the target itself. The
bigger issue is whether the player can realistically get there in time.

## Revised Balance Picture

With the current constants:

- `SCIENCE_RESEARCH_COST_MULTIPLIER = 500`
- `DIPLOMATIC_OFFER_COST = 500`
- `TRADE_RELATIONS_MULTIPLIER = 2`
- `RELATIONS_LOSS_PER_TURN = 3`
- `DIPLOMATIC_OVERTURE_CHANCE_PERCENT = 50`
- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT = 50`

the game no longer looks primarily blocked by expensive science or expensive
formal diplomacy. Those two systems are now much more accessible.

The main remaining risk is that campaigns become swingy rather than simply slow:

- strong starts may snowball earlier through cheap science
- relation growth may still feel grindy because trade gains are modest and decay
  is constant
- the final ally/colony conversion is still a coin flip
- sea risk may still suppress the very trade activity that diplomacy depends on
- a player who reaches gun science early may gain a disproportionate naval edge
  against baseline enemy fleets

In other words:

- science is now more likely to be an accelerator than a bottleneck
- diplomatic offer cost is now more likely to be fair than punitive
- relation pacing and expedition reliability remain the main balance concerns
- naval science is now a first-class balance concern rather than a secondary one

## Recommended Balance Strategy

Do not tune everything at once. Use a staged approach.

### Pass 1: make the mid game reachable

Change only:

- diplomacy pacing
- expedition danger
- maybe trader capacity or trader upkeep
- do not reduce science cost further in this pass

Goal:

- a strong player should be threatening a win by the second or third council

### Pass 2: make science timing matter

After mid-game diplomacy works, re-evaluate:

- `SCIENCE_RESEARCH_COST_MULTIPLIER`
- price spread constants if needed

Goal:

- science should accelerate or specialize a winning economy, but should not
  dominate all other strategic choices

### Pass 3: smooth late-game pressure

After the first two passes, tune:

- upkeep values
- battle-loss values
- bounty variance

Goal:

- late game should remain tense without collapsing into random punishment

## How To Model Balance Before More Code Changes

The game is a good candidate for Monte Carlo and heuristic simulation.

### Why simulation is worth doing

Manual playtesting will find obvious pain points, but not distribution-level
problems such as:

- how often the player goes bankrupt before turn `10`
- how often the first alliance appears before turn `15`
- whether one strategy dominates all others
- whether victory timing is too random

### What to simulate

A simulator does not need the full UI. It only needs:

- inventory state
- treasury
- workers, wagons, traders, warships
- science level
- relation values for each foreign nation
- market prices and battle events
- council outcomes every `10` turns

### Strategies to model

At minimum, simulate several policy archetypes:

- balanced: expands transport, production, traders, and diplomacy evenly
- trade-first: prioritizes traders and relation building
- science-first: buys research as early as possible
- navy-first: prioritizes warships for safer expeditions
- industrial-first: prioritizes workers and domestic production before diplomacy

If one policy beats the others almost every time, the game is not balanced yet.

### Metrics to track

For each run, record:

- turn of bankruptcy, if any
- turn of first `Great` relation
- turn of first ally/colony
- council vote totals at turns `10`, `20`, `30`, `40`
- turn of victory
- final treasury
- number of battles fought
- traders lost
- warships lost
- research levels achieved

Then review distributions, not just averages:

- win rate by turn band
- median and 90th percentile bankruptcy turn
- median alliance timing
- variance in final treasury

### Success criteria for the simulator

A promising balance state would look something like:

- low early bankruptcy rate for competent policies
- clear difference between strong and weak strategies
- no single dominant strategy with overwhelming win rate
- most wins clustering around council `2` or `3`
- late victories still possible, but not mandatory

## Recommended Next Step

Before changing any mechanics beyond constants, the most useful next step is:

1. decide whether the current worker-upkeep and naval-science behaviors are
   intentional
2. test the current build with the new cheaper science and overture costs before
   changing more constants
3. if diplomacy still feels slow, change `TRADE_RELATIONS_MULTIPLIER` or
   `RELATIONS_LOSS_PER_TURN` next, not `DIPLOMATIC_OFFER_COST`
4. if trade still feels too risky to support diplomacy, reduce expedition battle
   pressure next
5. specifically test whether early gun science makes small fleets too efficient
   in battle
6. build a simple off-engine simulator that mirrors the turn rules and runs
   large batches of games

If only one area is changed first, it should still be diplomacy pacing,
specifically the rate at which trade translates into lasting relation progress.
That is now the clearest remaining system-level limiter on satisfying mid- and
late-game play.

## Practical Next Steps

The next steps should be empirical, not speculative.

### 1. Playtest the new baseline

Run several games with the current values:

- `SCIENCE_RESEARCH_COST_MULTIPLIER = 500`
- `DIPLOMATIC_OFFER_COST = 500`

Track:

- turn of first research purchase
- turn of first `Great` relation
- turn of first ally/colony
- council votes at turns `10`, `20`, `30`
- whether losses at sea are stopping profitable trade loops
- whether gun-tech fleets are consistently beating larger baseline fleets

Goal:

- confirm whether the game now feels faster in a good way, or merely swingier

### 2. If diplomacy is still too slow, change only one pacing lever

Prefer this order:

1. `TRADE_RELATIONS_MULTIPLIER` from `2` to `3`
2. `RELATIONS_LOSS_PER_TURN` from `3` to `2`
3. `DIPLOMATIC_OVERTURE_CHANCE_PERCENT` from `50` to `60`

This keeps cause and effect clear during testing.

### 3. If trade is still too volatile, reduce expedition danger

Try one of:

- `TRADE_EXPEDITION_BATTLE_CHANCE_PERCENT` from `50` to `35`
- `TRADE_EXPEDITION_ATTACK_FOREIGN_NATION_CHANCE_PERCENT` from `50` to `30`

Goal:

- trade should remain risky, but should not be so unreliable that the diplomacy
  game cannot get moving

### 4. Specifically test naval science power

Because gun science now changes battle odds directly, test a few concrete cases:

- equal fleets with and without gun tech
- smaller player fleet with gun tech vs larger enemy fleet
- late-game battles after `Shell Guns`

Goal:

- science should create a meaningful combat edge
- it should not make ship count mostly irrelevant

If naval science proves too strong, the next solutions are likely to be:

- slowing research timing slightly
- increasing enemy fleet size scaling
- giving enemies improved gun baselines later in the game

### 5. Do not lower science cost further yet

At `500`, science is already much more available. Lowering it again before
re-testing diplomacy and trade would make it harder to tell whether future
improvements came from better pacing or from pure economic acceleration.

### 6. Build a simple simulator after the next tuning pass

Once one more round of diplomacy/trade tuning is done, build a lightweight
simulation harness and measure:

- bankruptcy rate before turn `10`
- alliance/colony timing
- win timing by council number
- win rate by strategic archetype
- battle outcomes by fleet-size and gun-tech combination

That will show whether the game is actually becoming better balanced or just
more generous.
