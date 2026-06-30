# actorsc

A deterministic, concurrent, **VHDL-style signal-driven actor engine** in C++20.

Actors (VHDL *processes*) read the **current** values of their input signals and
write **scheduled** values. The runtime advances in discrete **simulation
cycles**, each of which resolves all signal **transactions** to a fixed point
through zero-time **delta cycles** before time advances.

See [`PROJECT_PLAN.md`](PROJECT_PLAN.md) and [`CLAUDE.md`](CLAUDE.md) for the
authoritative architecture, terminology, and invariants.

## Layout

```
signals/     Signal<T>, SignalBase, SignalValue concept
actors/      ActorBase (process) + sensitivity list, ActorContext, ActorRegistry
scheduler/   ThreadPool, DeltaCycleEngine, SimulationCycleEngine
factory/     IoCContainer, AbstractFactory (-> ElaboratedDesign)
runtime/     Runtime (wires container + factory + engine), main.cpp demo
distributed/ node/transport/sync placeholders (future, see PROJECT_PLAN §5.2)
tests/       gtest: unit / functional / integration / distributed (scaffold)
```

## Build & test

Requires a C++20 compiler and CMake ≥ 3.16.

```sh
cmake -S . -B build -G Ninja        # or the default Make generator
cmake --build build
cd build && ctest --output-on-failure
```

Run the demo simulation (A → NOT → B → BUF → C):

```sh
./build/simulation
```

## Execution model

```
simulation cycle (time T):
  run every process once, then repeat delta cycles until stable:
    delta cycle:
      1. active processes execute  (read current, write scheduled)
      2. recompute transactions    (scheduled != current)
      3. queue processes sensitive to a transacting signal
      4. commit transacting signals (scheduled -> current)   <-- delta boundary
  advance time; apply external stimuli; repeat
```

## Deviations from the spec documents

Both are required for the engine to be correct and compilable; everything else
follows `PROJECT_PLAN.md` / `CLAUDE.md` exactly (terminology, module layout,
signal/actor/scheduler/IoC structure, invariants, test taxonomy).

1. **Header-only (`.hpp`) instead of C++20 named modules (`.cppm`).**
   The spec asks for `.cppm` modules. Named modules currently require
   `clang-scan-deps` + Ninja ≥ 1.11 under CMake, which is not available in every
   toolchain. Using headers keeps the *exact same* module boundaries and public
   API while building on any C++20 compiler with stock CMake. Each spec module
   maps 1:1 to a header (e.g. `signals.base` → `signals/signal_base.hpp`).

2. **Signals commit per delta cycle, not only at the simulation-cycle boundary.**
   The PROJECT_PLAN diagram shows `commit scheduled → current` only after delta
   cycles are "exhausted". Taken literally, a signal with `scheduled != current`
   keeps its transaction forever, so the delta loop never terminates. IEEE-1076
   updates signals at *every* delta-cycle boundary; that is what lets values
   propagate through the graph and reach a fixed point. The "current value is
   constant during a delta cycle" invariant still holds: current is frozen
   during process execution and updated only at the delta boundary.
   `DeltaCycleEngine` also enforces a `kMaxDeltaIterations` guard that throws on
   a non-converging (oscillating) graph instead of hanging.
