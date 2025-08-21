# iimperialism
Apple II game inspired by Imperialism and Taipan!

## Overview

The game is turn-based. Each turn:

1. **Provinces produce raw materials** (Timber and Cotton).
2. The player’s **transport orders** bring some of those resources into the warehouse.
3. The player’s **production orders** convert raw materials into finished goods.
4. The warehouse shows updated stocks.
5. The date advances by 7 days.

The challenge is to manage resources efficiently across six commodities:
- **Timber → Lumber → Furniture**
- **Cotton → Fabric → Clothes**

---

## Game Logic

### Data Structures

- **Warehouse stocks**
  - `TI` = Timber  
  - `LU` = Lumber  
  - `FU` = Furniture  
  - `CO` = Cotton  
  - `FA` = Fabric  
  - `CH` = Clothes  

- **Orders for next turn**
  - `TT` = Timber transport  
  - `TC` = Cotton transport  
  - `PL` = Sawmill orders (2 Timber → 1 Lumber)
  - `PF` = Furniture Factory orders (2 Lumber → 1 Furniture)
  - `PG` = Textile Workshop orders (2 Cotton → 1 Fabric)  
  - `PC` = Clothes Factory orders (2 Fabric → 1 Clothes)

- **Transport capacity**
  - `TCAP` = Maximum total transport orders allowed per turn (starts at 20)
  - Transport orders (`TT` + `TC`) cannot exceed transport capacity
  - Players can increase capacity through the Transport Orders menu

- **Factory capacities**
  - `SCAP` = Sawmill capacity (starts at 1)
  - `FCAP` = Furniture Factory capacity (starts at 1)
  - `WCAP` = Textile Workshop capacity (starts at 1)
  - `CCAP` = Clothes Factory capacity (starts at 1)
  - Each factory can only process up to its capacity per turn
  - Players can increase individual factory capacities  

- **Province supply (changes every turn)**
  - `ST` = Timber supply available in provinces this turn  
  - `SC` = Cotton supply available in provinces this turn  

### Turn Cycle

1. **Transport Phase**
   - Orders `TT` and `TC` are capped at available provincial supply `ST` and `SC`.
   - Total transport orders (`TT` + `TC`) cannot exceed transport capacity `TCAP`.
   - Result is added to warehouse (`TI` and `CO`).

2. **Production Phase**
   - **Sawmill** (`PL`): Each order consumes **2 Timber → 1 Lumber** (capacity limited)
   - **Furniture Factory** (`PF`): Each order consumes **2 Lumber → 1 Furniture** (capacity limited)
   - **Textile Workshop** (`PG`): Each order consumes **2 Cotton → 1 Fabric** (capacity limited)
   - **Clothes Factory** (`PC`): Each order consumes **2 Fabric → 1 Clothes** (capacity limited)
   - Orders are reduced if insufficient resources or factory capacity available.

3. **Date Advancement**
   - The date advances by 7 days each turn.
   - Handles month changes and leap years.

4. **Reset Orders**
   - After processing, all orders reset to zero.

---

## Code Structure

- **2000–2150**: Screen rendering (warehouse, orders, prompt).
- **3000–3060**: Input handler (`T`, `P`, `N`).
- **4000–4100**: Enter transport orders.
- **4500–4610**: Enter production orders.
- **5000–5010**: Reset orders to zero.
- **6000–6030**: Generate random provincial supply each turn (5–20 units of each raw good).
- **7000–7170**: Process transport and production logic.
- **8000–8070**: Advance date by 7 days, including month/year rollover.
- **8500–8520**: Month length calculation (handles leap years).
- **9000–9010**: Print header line with date.

---

## How to Play

1. **Start the program**: type `RUN`.
2. Each turn, review warehouse stocks and decide:
   - Do I need more raw materials? (use **Transport**).
   - Do I want to manufacture goods? (use **Production**).
   - Or am I ready to move time forward? (use **Next turn**).
3. **Transport Orders screen** (press **T**):
   - View current transport capacity and orders
   - **A** = Add transport capacity
   - **T** = Change transport orders (validates against capacity)
   - **C** = Cancel and return
4. **Production Orders screen** (press **P**):
   - View all four factories with individual capacities and orders
   - **C** = Change production orders (validates against factory capacities)
   - **A** = Add capacity to specific factories
   - **R** = Return to main screen
5. Enter orders carefully—resources are limited by supply, transport capacity, factory capacity, and conversion costs.
6. Press **N** to end the turn and see the results.

---
