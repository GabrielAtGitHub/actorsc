# actorsc

[![CI](https://github.com/GabrielAtGitHub/actorsc/actions/workflows/ci.yml/badge.svg)](https://github.com/GabrielAtGitHub/actorsc/actions/workflows/ci.yml)

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

## Continuous integration

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) builds and runs the full
test suite on every push and pull request.

To run the same gate locally before each commit, enable the hook:

```sh
git config core.hooksPath .githooks    # build + test on `git commit`
git commit --no-verify                 # bypass once if needed
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

## Spec conformance

The implementation follows [`PROJECT_PLAN.md`](PROJECT_PLAN.md) and
[`CLAUDE.md`](CLAUDE.md) — terminology, component layout, signal / actor /
scheduler / IoC structure, formal invariants, and test taxonomy.

Two things worth knowing:

- **Header-only, not C++20 named modules.** The engine is `.hpp`-based so it
  builds on any C++20 compiler with CMake ≥ 3.16 and gives reliable IDE
  tooling (type/call hierarchy, navigation). Named modules are deferred until
  the toolchain matures — see *Appendix: C++20 Modules (Deferred)* in
  PROJECT_PLAN.md.
- **Convergence guard.** `DeltaCycleEngine` enforces a `kMaxDeltaIterations`
  limit and throws on a non-converging (oscillating) actor graph rather than
  looping forever.
