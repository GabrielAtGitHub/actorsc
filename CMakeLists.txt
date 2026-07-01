cmake_minimum_required(VERSION 3.16)
project(actorsc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Emit DWARF 4 in Debug builds. Clang >= 14 defaults to DWARF 5, which the
# system gdb on Ubuntu 20.04 (9.2) cannot parse — line tables are unreadable,
# so source-level breakpoints never bind. DWARF 4 is read fine by old gdb and
# is harmless on newer toolchains. Applies to Clang and GCC.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options($<$<CONFIG:Debug>:-gdwarf-4>)
endif()

find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# The engine is header-only. Each spec component is one header; the project
# root is the single include root so headers are included as, e.g.,
#   #include "signals/signal_base.hpp"
# See PROJECT_PLAN.md "Appendix: C++20 Modules (Deferred)" for why named
# modules are not used yet.
# ---------------------------------------------------------------------------
set(ACTORSC_INCLUDE_DIR ${CMAKE_SOURCE_DIR})

# signals (header-only)
add_library(signals INTERFACE)
target_include_directories(signals INTERFACE ${ACTORSC_INCLUDE_DIR})

# actors (depends on signals)
add_library(actors INTERFACE)
target_include_directories(actors INTERFACE ${ACTORSC_INCLUDE_DIR})
target_link_libraries(actors INTERFACE signals)

# scheduler (depends on actors + signals + threads)
add_library(scheduler INTERFACE)
target_include_directories(scheduler INTERFACE ${ACTORSC_INCLUDE_DIR})
target_link_libraries(scheduler INTERFACE actors signals Threads::Threads)

# factory (depends on scheduler + actors + signals)
add_library(factory INTERFACE)
target_include_directories(factory INTERFACE ${ACTORSC_INCLUDE_DIR})
target_link_libraries(factory INTERFACE scheduler actors signals)

# runtime library (depends on factory + scheduler)
add_library(runtime_lib INTERFACE)
target_include_directories(runtime_lib INTERFACE ${ACTORSC_INCLUDE_DIR})
target_link_libraries(runtime_lib INTERFACE factory scheduler actors signals)

# simulation executable
add_executable(simulation runtime/main.cpp)
target_link_libraries(simulation PRIVATE runtime_lib)

# distributed stubs (header-only, standalone)
add_library(distributed INTERFACE)
target_include_directories(distributed INTERFACE ${ACTORSC_INCLUDE_DIR})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
