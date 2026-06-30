cmake_minimum_required(VERSION 3.28)
project(actorsc CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# C++20 named modules require Clang >= 17, Ninja >= 1.11, clang-scan-deps, and
# CMake >= 3.28. Configure with, e.g.:
#   cmake -DCMAKE_CXX_COMPILER=clang++-18 -S . -B build -G Ninja

# ---------------------------------------------------------------------------
# signals
# ---------------------------------------------------------------------------
add_library(signals)
target_sources(signals
    PUBLIC FILE_SET CXX_MODULES FILES
        signals/signal_concepts.cppm
        signals/signal_base.cppm
        signals/signal_bool.cppm
        signals/signal_int.cppm
)

# ---------------------------------------------------------------------------
# actors
# ---------------------------------------------------------------------------
add_library(actors)
target_sources(actors
    PUBLIC FILE_SET CXX_MODULES FILES
        actors/actor_context.cppm
        actors/actor_base.cppm
        actors/actor_registry.cppm
)
target_link_libraries(actors PUBLIC signals)

# ---------------------------------------------------------------------------
# scheduler
# ---------------------------------------------------------------------------
add_library(scheduler)
target_sources(scheduler
    PUBLIC FILE_SET CXX_MODULES FILES
        scheduler/thread_pool.cppm
        scheduler/delta_cycle.cppm
        scheduler/simulation_cycle.cppm
)
target_link_libraries(scheduler PUBLIC actors signals Threads::Threads)

# ---------------------------------------------------------------------------
# factory
# ---------------------------------------------------------------------------
add_library(factory)
target_sources(factory
    PUBLIC FILE_SET CXX_MODULES FILES
        factory/ioc_container.cppm
        factory/abstract_factory.cppm
)
target_link_libraries(factory PUBLIC scheduler actors signals)

# ---------------------------------------------------------------------------
# runtime library + simulation executable
# ---------------------------------------------------------------------------
add_library(runtime_lib)
target_sources(runtime_lib
    PUBLIC FILE_SET CXX_MODULES FILES
        runtime/elaborated_design.cppm
)
target_link_libraries(runtime_lib PUBLIC factory scheduler actors signals)

add_executable(simulation runtime/main.cpp)
target_link_libraries(simulation PRIVATE runtime_lib)

# ---------------------------------------------------------------------------
# distributed (stubs, standalone)
# ---------------------------------------------------------------------------
add_library(distributed)
target_sources(distributed
    PUBLIC FILE_SET CXX_MODULES FILES
        distributed/node.cppm
        distributed/transport.cppm
        distributed/sync.cppm
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
