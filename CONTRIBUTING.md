# Contributing to actorsc

Thanks for contributing! This project is a VHDL-style, signal-driven actor
engine in C++20. Please keep changes aligned with the authoritative specs:
[`PROJECT_PLAN.md`](PROJECT_PLAN.md) and [`CLAUDE.md`](CLAUDE.md).

## Toolchain

The engine is header-only, so the toolchain is conservative (see
[`DEV_ENVIRONMENT.md`](DEV_ENVIRONMENT.md) for details):

- Any C++20 compiler (verified with **clang-18**; GCC ≥ 10 also works)
- CMake ≥ 3.16, any generator (Ninja recommended)

```sh
sudo apt-get install -y clang-18 lld-18 cmake ninja-build
```

C++20 named modules are deferred — see PROJECT_PLAN.md
"Appendix: C++20 Modules (Deferred)".

## Build & test

```sh
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DCMAKE_C_COMPILER=clang-18
cmake --build build
ctest --test-dir build --output-on-failure --timeout 60
```

Optionally enable the local gate so each commit builds and tests first:

```sh
git config core.hooksPath .githooks    # runs build + test on `git commit`
git commit --no-verify                 # bypass once if needed
```

## Branch & PR workflow

`main` is protected and only changes via pull request. CI
([`.github/workflows/ci.yml`](.github/workflows/ci.yml)) must pass before a PR
can merge.

Branch model:

| Branch | Purpose |
|--------|---------|
| `main` | stable; merge only via green PR |
| `dev` | active development |
| `feature/*` | new modules or enhancements |
| `test/*` | testing improvements |

Typical flow:

```sh
git checkout -b feature/my-change
# ...edit, build, test...
git commit            # pre-commit hook builds + tests if enabled
git push -u origin feature/my-change
# open a PR; wait for CI green; merge
```

## Coding guidelines

- **Match the spec terminology** (VHDL: process/actor, signal, transaction,
  delta cycle, simulation cycle). Do not introduce "effective/projected" terms.
- **One component per header** (`.hpp`, `#pragma once`), included by
  project-root-relative path. Keep the include graph acyclic.
- **Keep the formal invariants** (PROJECT_PLAN §13 / CLAUDE §10). In
  particular, signal `current` is constant during process execution and updated
  at the delta-cycle boundary.
- **Add tests** in the matching tier: `tests/unit`, `tests/functional`,
  `tests/integration` (distributed tests stay scaffolded/`DISABLED_`).
- Follow the surrounding code style (LLVM-ish; `clang-format` fallback style is
  LLVM).

## Commit messages

Short imperative subject, optional body explaining the *why*. Example:

```
scheduler: add work-stealing to the thread pool

Reduces idle time when actor execution times are uneven.
```

## Reporting issues

Open a GitHub issue with: what you expected, what happened, and the minimal
steps (ideally a failing test) to reproduce.
