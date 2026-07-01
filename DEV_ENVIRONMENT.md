---

# **DEV_ENVIRONMENT.md**  
## **Development Environment Setup for Actor‑Model, VHDL‑Style Execution Engine (C++20)**

---

## **1. Overview**
This document describes the **development environment** required to build, test, and run the C++20 modular actor‑model system defined in `PROJECT_PLAN.md`.

It covers:

- WSL2 setup  
- VS Code configuration  
- CMake + Ninja build system  
- Compiler requirements  
- Debugging tools  
- Profiling tools  
- Recommended extensions  
- Repository layout expectations  

This file does **not** define architecture or module semantics.  
Those are defined in `PROJECT_PLAN.md` and `CLAUDE.md`.

---

## **2. System Requirements**

### **2.1 Operating System**
- Windows 11  
- WSL2 (Ubuntu recommended)

### **2.2 Compiler**
You must use a compiler with **full C++20 module support**:

- **Clang ≥ 17** (recommended; this project is verified with **Clang 18**)  
- **GCC ≥ 14** (partial module support; improving)  
- **MSVC ≥ VS2022 17.10** (good module support)

Clang is recommended for consistency across WSL2.

> **C++20 modules require `clang-scan-deps`.** CMake uses it to scan the
> `.cppm` files and build the module dependency graph. On Debian/Ubuntu this
> tool is **not** in the `clang-NN` or `llvm-NN-tools` packages — it ships in
> **`clang-tools-NN`** (e.g. `clang-tools-18`). Without it, configuration fails
> with `CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS-NOTFOUND`. See §4.2.

### **2.3 Build System**
- **CMake ≥ 3.28** (stable C++20 module support; verified with **CMake 4.3**)  
- **Ninja ≥ 1.11** (required for module `dyndep`; fast incremental builds)

> The Ninja shipped by Ubuntu 20.04 apt is **1.10**, which is too old for
> modules (`The Ninja generator does not support C++20 modules using Ninja
> version 1.10.0`). Install a newer Ninja with `pip` (see §4.2).

### **2.4 Tools**
- `clang-scan-deps` (**required** for modules — from `clang-tools-NN`)  
- `gdb` or `lldb` (this project's `.vscode` debug configs use **gdb**)  
- `clang-tidy`  
- `clang-format`  
- `valgrind`  
- `perf`  
- `heaptrack`  

---

## **3. Repository Layout**

Your repository should follow this structure:

```
/project-root
    /.vscode            # editor/build/run/test/debug config (§5)
    /signals
    /actors
    /scheduler
    /factory
    /runtime
    /distributed
    /tests              # unit / functional / integration / distributed
    CMakeLists.txt
    README.md
    PROJECT_PLAN.md
    CLAUDE.md
    DEV_ENVIRONMENT.md
```

Module files use `.cppm` (or `.ixx` if you choose that convention). Each
spec module maps to one `.cppm` (e.g. `signals.base` →
`signals/signal_base.cppm`). Note `bool`/`int` are keywords, so those two
signal modules are named `signals.signal_bool` / `signals.signal_int`.

---

## **4. WSL2 Setup**

### **4.1 Install WSL2**
From PowerShell:

```
wsl --install
```

### **4.2 Install Ubuntu packages**
Inside WSL:

```
sudo apt update
sudo apt install -y \
    build-essential \
    clang-18 \
    clang-tools-18 \
    lld-18 \
    gdb \
    valgrind \
    linux-tools-common \
    linux-tools-generic \
    linux-tools-$(uname -r) \
    heaptrack
```

> **`clang-tools-18` is mandatory** — it provides `clang-scan-deps`, without
> which CMake cannot build the `.cppm` modules.

#### Newer CMake + Ninja via pip

The apt `cmake` (3.16 on 20.04) and `ninja-build` (1.10) are too old for
modules. Install up-to-date versions with `pip` (they land in `~/.local/bin`,
so make sure that is on your `PATH`):

```
pip3 install --upgrade cmake ninja
```

Verify:

```
cmake --version    # >= 3.28
ninja --version    # >= 1.11
clang-scan-deps-18 --version
```

### **4.3 Set Clang as default**
Optional:

```
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/clang-17 100
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-17 100
```

---

## **5. VS Code Setup**

The repository ships a ready-to-use `.vscode/` configuration. Open the folder
in VS Code (WSL mode) and accept the recommended extensions — editing,
building, running, testing, and debugging all work out of the box.

### **5.1 Recommended Extensions**
These are listed in `.vscode/extensions.json` and VS Code will offer to install
them automatically:

- **C/C++** (`ms-vscode.cpptools`) — IntelliSense + debugging  
- **C/C++ Extension Pack** (`ms-vscode.cpptools-extension-pack`)  
- **CMake Tools** (`ms-vscode.cmake-tools`)  
- **CMake** (`twxs.cmake`) — syntax highlighting  
- **C++ TestMate** (`matepek.vscode-catch2-test-adapter`) — gtest in the Testing panel  

For WSL also install **WSL** / **Remote Development**, plus **GitLens** if you
want richer Git history.

### **5.2 What the committed config does**

| File | Role |
|------|------|
| `settings.json` | CMake Tools defaults (Ninja, `Debug`, `clang++-18`/`clang-18`, `CMAKE_EXPORT_COMPILE_COMMANDS`), IntelliSense compiler/standard, `*.cppm` → C++ association, format-on-save. Prepends `~/.local/bin` to `PATH` so the pip-installed Ninja is found. |
| `c_cpp_properties.json` | Points IntelliSense at `build/compile_commands.json` so per-file **module** flags resolve correctly (C++20, clang mode). |
| `tasks.json` | `configure`, `build` (default build task), `build: simulation`, `clean rebuild`, `run: simulation`, `test`. Each is a fully-specified `cmake`/`ctest` call, so they work with or without the CMake Tools extension. |
| `launch.json` | gdb debug configs: **Debug simulation**, **Debug active test file** (debugs the executable matching the open `*.cpp`), **Debug test (pick)** (dropdown of all test executables). |

IntelliSense relies on `build/compile_commands.json`, which is produced by the
first configure. If symbols look unresolved, run the **configure** task once.

### **5.3 Day-to-day workflow**

- **Build:** `Ctrl+Shift+B` (runs the `build` task; it configures first if needed).  
- **Run:** *Terminal → Run Task… → `run: simulation`*.  
- **Test:** *Run Test Task* (`test`), or use the Testing panel via C++ TestMate.  
- **Debug:** press **F5** and pick a configuration. The 3 disabled
  `DistributedScaffold` tests are skipped by gtest automatically.  
- **Clean:** run the `clean rebuild` task to wipe `build/` and reconfigure
  (useful after large module-graph changes).

### **5.4 Debugger note**
The launch configs use **gdb** (`/usr/bin/gdb`, `MIMode: gdb`). If you prefer
`lldb`, install `lldb-18` and change `MIMode`/`miDebuggerPath` in
`.vscode/launch.json`. Builds use `CMAKE_BUILD_TYPE=Debug`, so binaries carry
debug symbols.

---

## **6. Building the Project**

> From VS Code, just use the tasks in §5.3. The commands below are the
> equivalent CLI invocations.

### **6.1 Configure**
```
cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DCMAKE_C_COMPILER=clang-18
```

### **6.2 Build**
```
cmake --build build
```

### **6.3 Run Tests**
```
ctest --test-dir build --output-on-failure --timeout 30
```

The 3 `DistributedScaffold` tests are `DISABLED_` placeholders (PROJECT_PLAN
§7.4) and are skipped automatically.

---

## **7. Running the Simulation**

After building:

```
./build/simulation
```

This runs the main simulation loop:

- Delta cycles  
- Simulation cycles  
- Fixed‑point convergence  

---

## **8. Code Quality Tools**

### **8.1 clang‑format**
Format all code:

```
clang-format -i $(find . -name "*.cpp" -o -name "*.hpp" -o -name "*.cppm")
```

### **8.2 clang‑tidy**
Run static analysis:

```
clang-tidy --config-file=.clang-tidy
```

### **8.3 Sanitizers**
Add to CMake:

```
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined")
```

---

## **9. Profiling**

### **9.1 perf**
```
perf record ./build/runtime/simulation
perf report
```

### **9.2 heaptrack**
```
heaptrack ./build/runtime/simulation
heaptrack_gui heaptrack.out
```

### **9.3 valgrind**
```
valgrind --leak-check=full ./build/runtime/simulation
```

---

## **10. Distributed Runtime (Future)**

When distributed execution is implemented:

- Install ZeroMQ or gRPC  
- Add cluster orchestration tools  
- Add distributed tracing (Jaeger/OpenTelemetry)  
- Add network profiling tools  

This section will expand once the distributed module is implemented.

---

## **11. Developer Workflow**

### **11.1 Typical workflow**
1. Clone repo  
2. Open in VS Code (WSL mode)  
3. Configure CMake  
4. Build  
5. Run tests  
6. Run simulation  
7. Profile if needed  
8. Commit + push  

### **11.2 Branching model**
- `main` — stable  
- `dev` — active development  
- `feature/*` — new modules or enhancements  
- `test/*` — testing improvements  

---

## **12. Troubleshooting**

### **12.1 CMake cannot find Clang**
Set compiler explicitly:

```
cmake -DCMAKE_CXX_COMPILER=clang++-18 -DCMAKE_C_COMPILER=clang-18 -S . -B build -G Ninja
```

### **12.2 `CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS-NOTFOUND`**
`clang-scan-deps` is missing. Install the `clang-tools` package matching your
Clang version (it is **not** in `clang-NN` or `llvm-NN-tools`):

```
sudo apt install -y clang-tools-18
clang-scan-deps-18 --version   # confirm
```

Then delete `build/` and reconfigure.

### **12.3 "Ninja generator does not support C++20 modules (version 1.10.0)"**
Your Ninja is too old. Modules need **≥ 1.11**:

```
pip3 install --upgrade ninja
ninja --version                # >= 1.11, from ~/.local/bin
```

Ensure `~/.local/bin` precedes `/usr/bin` on `PATH` so the pip Ninja wins.

### **12.4 IntelliSense can't resolve `import` / module symbols**
IntelliSense reads `build/compile_commands.json`. Run the **configure** task
(or the §6.1 command) once to generate it, then reload the window.

### **12.5 VS Code not attaching debugger**
Ensure the WSL extension is installed and the remote session is active. The
launch configs expect `gdb` at `/usr/bin/gdb` (`sudo apt install -y gdb`).

### **12.6 Breakpoints don't bind / debugger stops at an address, not a line**
Symptom: gdb stops like `Breakpoint 1, 0x... in main ()` with no
`at main.cpp:NN`, and source breakpoints never suspend. Cause: Clang ≥ 14
emits **DWARF 5**, which the Ubuntu 20.04 system **gdb 9.2** cannot read.
The build sets `-gdwarf-4` for Debug (see `CMakeLists.txt`) to fix this — make
sure you are on a **Debug** build and have rebuilt. Verify:

```
readelf --debug-dump=info build/simulation | grep -m1 Version   # expect 4
gdb -batch -ex 'info line main' build/simulation                 # expect a main.cpp line
```

Alternatively, use a newer gdb (≥ 10, ideally 12+) or `lldb-18`, both of which
read DWARF 5.

### **12.7 Wrong CMake kit (clang-cl)**
If configure fails with `cannot find -libpath:lib/amd64`, CMake Tools selected
the **clang-cl** (MSVC-style) kit. Pick **CMake: Select a Kit → Clang …
x86_64-pc-linux-gnu**, then **CMake: Delete Cache and Reconfigure**.

### **12.8 Generic Ninja build errors**
Clean build:

```
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++-18 -DCMAKE_C_COMPILER=clang-18
```

---
