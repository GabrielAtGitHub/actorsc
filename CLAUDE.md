---

# **CLAUDE.md**  
## **Specification for Claude Code Integration**  
### **Actor‑Model, Signal‑Driven, VHDL‑Style Execution Engine (C++20)**

---

## **1. Purpose of This Document**
This file instructs **Claude Code** how to:

- Generate code consistent with the **PROJECT_PLAN.md**  
- Follow the **VHDL‑style execution cycle**  
- Use the **modular header layout** (C++20 named modules deferred — see Appendix A)  
- Respect **IoC + abstract factory instantiation**  
- Produce **deterministic, concurrent actor code**  
- Generate **unit, functional, and integration tests**  
- Prepare for **distributed scaling**  

Claude must treat this file as the **authoritative specification** for all code generation.

---

## **2. Architectural Model (Authoritative)**

### **2.1 VHDL‑Aligned Concepts**
Claude must use VHDL terminology:

| VHDL Term | Meaning in This Project |
|----------|--------------------------|
| **Process** | Actor |
| **Signal** | Typed value with current/scheduled semantics |
| **Current value** | Value visible during delta cycle |
| **Scheduled value** | Value computed during delta cycle |
| **Transaction** | scheduled ≠ current |
| **Delta cycle** | Zero‑time event resolution cycle |
| **Simulation cycle** | Time step |
| **Sensitivity list** | Actor input signal list |

Claude must **never** use “effective/projected” terminology.  
Only **current/scheduled/transaction**.

---

## **3. Required Component Structure**

The engine is currently **header-only** (`.hpp`). Each architectural component
is one header; C++20 *named modules* (`.cppm`) are deferred until the toolchain
matures — see **Appendix A: C++20 Modules (Deferred)** and PROJECT_PLAN.md for
the rationale. Claude must generate code using the following structure:

```
signals/
    signal_base.hpp
    signal_bool.hpp
    signal_int.hpp
    signal_concepts.hpp

actors/
    actor_base.hpp
    actor_context.hpp
    actor_registry.hpp

scheduler/
    delta_cycle.hpp
    simulation_cycle.hpp
    thread_pool.hpp

factory/
    ioc_container.hpp
    abstract_factory.hpp
    workflow_loader.hpp   (future)

runtime/
    main.cpp
    elaborated_design.hpp

distributed/ (future)
    node.hpp
    transport.hpp
    sync.hpp

tests/
    unit/
    functional/
    integration/
    distributed/          (scaffold only)
```

Headers use `#pragma once` and include siblings by project-root-relative path,
e.g. `#include "signals/signal_base.hpp"`.

Claude must **not** invent additional modules unless explicitly instructed.

---

## **4. Signal Specification (Authoritative)**

Claude must generate signals with:

```cpp
template <SignalValue T>
struct Signal {
    T current_value;
    T scheduled_value;
    bool has_transaction;
};
```

Required operations:

- `read_current()`
- `write_scheduled()`
- `compute_transaction()`
- `commit_scheduled()`

Transactions must be computed as:

```
has_transaction = (scheduled_value != current_value)
```

---

## **5. Actor (Process) Specification**

Claude must generate actors with:

- A **sensitivity list** (vector of pointers to signals)
- A **process body** that:
  - Reads **current_value**
  - Writes **scheduled_value**
  - Performs no side effects outside signal updates

Actor execution signature:

```cpp
void execute(ActorContext& ctx);
```

Actors must be instantiated via **IoC + abstract factory**.

---

## **6. Scheduler Specification**

Claude must implement:

### **6.1 Delta Cycle Engine**
- Execute active processes concurrently  
- Compute signal transactions  
- Activate sensitive processes  
- Commit scheduled → current at the delta‑cycle boundary  
- Repeat until stable (no active processes remain)  

### **6.2 Simulation Cycle Engine**
- Re‑run all processes once, then delta‑resolve to a fixed point  
- Advance time  
- Apply external stimuli to determine the next active processes  

Claude must follow the exact VHDL semantics:

```
delta cycles resolve all transactions before time advances
```

---

## **7. Inversion of Control (IoC)**

Claude must generate:

- A registry of actor types  
- A registry of signal types  
- A dependency graph builder  
- An abstract factory that produces a **fully elaborated design**  

The elaborated design must contain:

- All actors  
- All signals  
- All sensitivity bindings  
- A scheduler instance  
- A runtime instance  

---

## **8. Workflow‑Driven Instantiation (Future)**

Claude must prepare for:

- YAML/JSON workflow descriptions  
- Dynamic actor instantiation  
- Dynamic signal instantiation  
- Graph validation  

Claude must not implement workflow parsing unless explicitly asked.

---

## **9. Testing Requirements**

Claude must generate **gtest** suites:

### **9.1 Unit Tests**
- Signals  
- Actors  
- Scheduler primitives  
- IoC container  
- Abstract factory  

### **9.2 Functional Tests**
- Multi‑actor scenarios  
- Delta cycle transitions  
- Transaction propagation  
- Fixed‑point convergence  

### **9.3 Integration Tests**
- Full elaborated design  
- End‑to‑end simulation cycles  
- Concurrency correctness  

### **9.4 Distributed Tests (Future)**  
Claude must scaffold but not implement distributed tests.

---

## **10. Formal Invariants (Claude Must Enforce)**

Claude must ensure all generated code respects:

### **10.1 Signal Invariants**
- Current value is constant during process execution within a delta cycle; it is updated at the delta‑cycle boundary  
- Transactions exist iff scheduled ≠ current  
- Commit correctness (applied at each delta boundary):  
  \[
  current_{T+\Delta} = scheduled_T
  \]

### **10.2 Process Invariants**
- Pure function of current values + internal state  
- Activated iff sensitive signal has transaction  
- No wall‑clock time dependence  

### **10.3 Delta Cycle Invariants**
- Activation set is monotonic  
- Termination when no transactions exist  
- Deterministic execution  

### **10.4 Simulation Cycle Invariants**
- Each cycle reaches a fixed point  
- All signals globally consistent at commit  
- No sensitive process is missed  

Claude must generate assertions where appropriate.

---

## **11. Mermaid Diagrams (Claude Must Use for Documentation)**

### **Execution Cycle**

```mermaid
flowchart TD

    A["Start Simulation Cycle (Time T)"] --> B[Execute Active Processes]
    B --> C[Compute Signal Transactions]
    C --> D[Activate Sensitive Processes]
    D --> E[Commit scheduled → current at delta boundary]
    E --> F{Any Active Processes?}

    F -->|Yes| B
    F -->|No| G[Fixed Point Reached]
    G --> H[Advance Simulation Time]
    H --> I{Any External Stimuli?}

    I -->|Yes| A
    I -->|No| J[Simulation Complete]
```

### **Module Relationships**

```mermaid
flowchart LR

    subgraph SignalsModule[signals]
        SIG[Signal types & concepts]
    end

    subgraph ActorsModule[actors]
        ACT[Process/Actor interface]
    end

    subgraph SchedulerModule[scheduler]
        SCH[Delta & Simulation Cycle Engine]
    end

    subgraph FactoryModule[factory]
        FAC[IoC & Abstract Factory]
    end

    subgraph RuntimeModule[runtime]
        RUN[Main Simulation Loop]
    end

    subgraph DistributedModule[distributed]
        DIST[Multi-node Coordination]
    end

    subgraph TestsModule[tests]
        TST[gtest Suites]
    end

    FAC --> ACT
    FAC --> SIG

    SCH --> ACT
    SCH --> SIG

    RUN --> SCH
    RUN --> FAC

    DIST --> SCH
    DIST --> ACT
    DIST --> SIG

    TST --> ACT
    TST --> SIG
    TST --> SCH
    TST --> FAC
    TST --> RUN
```

---

## **12. Claude Code Behavior Requirements**

Claude must:

- Follow this specification **exactly**  
- Never invent architecture not defined here  
- Never rename concepts  
- Always use VHDL terminology  
- Always generate deterministic code  
- Always generate modular C++20 code  
- Always generate tests aligned with invariants  
- Always generate documentation consistent with diagrams  

Claude must treat this file as **the authoritative source of truth**.

---

## **13. Output Requirements**

Claude must generate:

- Modular C++20 headers (`.hpp`; named modules deferred — Appendix A)  
- IoC container  
- Abstract factory  
- Scheduler  
- Runtime  
- Tests  
- Documentation  

Claude must not generate:

- Build systems not requested  
- Distributed runtime unless asked  
- Workflow parser unless asked  

---

## **14. Final Directive**

Claude must use **PROJECT_PLAN.md** + **CLAUDE.md** together as the **complete specification** for all code generation.

Claude must not deviate from these documents without approval.

---

## **Appendix A: C++20 Modules (Deferred)**

The architecture is designed around module-sized components, and the codebase
was briefly implemented with C++20 **named modules** (`.cppm`). It has been
converted to **header-only** (`.hpp`) and named modules are **deferred** until
the surrounding toolchain matures.

**Why deferred:**

- **IDE/tooling immaturity.** IntelliSense for named modules is unreliable in
  2026: the C/C++ (cpptools) engine only partially supports `import` and emits
  false errors; clangd requires `--experimental-modules-support` and its
  cross-file **index** (type hierarchy, call hierarchy, find-references) does
  not fully cover module symbols.
- **BMI incompatibility.** Prebuilt module interfaces (`.pcm`) are not portable
  across compiler major versions, so an editor's clangd (e.g. 22) cannot read
  BMIs produced by the build compiler (clang-18) — `ast_file_version_too_old`.
- **Heavier toolchain floor.** Named modules require `clang-scan-deps`
  (`clang-tools-*`), Ninja ≥ 1.11, and CMake ≥ 3.28. Headers build with any
  C++20 compiler and CMake ≥ 3.16.
- **Keyword collisions.** `bool`/`int` cannot be module-name components, forcing
  awkward names like `signals.signal_bool`.

**What is preserved for a future modules migration:**

- One component per file with clean, acyclic dependencies — each `.hpp` maps
  1:1 to a future `.cppm` (e.g. `signals/signal_base.hpp` → `signals.base`).
- The public API, VHDL terminology, and all formal invariants are unchanged.

**Revisit when:** cpptools *or* clangd provides stable named-module IntelliSense
(including index-backed type/call hierarchy) and BMIs are version-tolerant, or
the editor clangd version tracks the build compiler.

---
