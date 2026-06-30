include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

function(add_actorsc_test target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE ${ARGN} GTest::gtest_main)
    gtest_discover_tests(${target})
endfunction()

# Unit tests
add_actorsc_test(test_signals   unit/test_signals.cpp   signals)
add_actorsc_test(test_actors    unit/test_actors.cpp    actors signals)
add_actorsc_test(test_thread_pool unit/test_thread_pool.cpp scheduler)
add_actorsc_test(test_ioc       unit/test_ioc.cpp       factory actors signals)
add_actorsc_test(test_factory   unit/test_factory.cpp   factory scheduler actors signals)

# Functional tests
add_actorsc_test(test_delta_cycle
    functional/test_delta_cycle.cpp
    scheduler actors signals)
add_actorsc_test(test_transaction_propagation
    functional/test_transaction_propagation.cpp
    scheduler actors signals)
add_actorsc_test(test_convergence
    functional/test_convergence.cpp
    factory scheduler actors signals)

# Integration tests
add_actorsc_test(test_elaborated_design
    integration/test_elaborated_design.cpp
    runtime_lib)
add_actorsc_test(test_simulation_cycle
    integration/test_simulation_cycle.cpp
    runtime_lib)

# Distributed tests (scaffold only — see PROJECT_PLAN.md §7.4)
add_actorsc_test(test_distributed_scaffold
    distributed/test_distributed_scaffold.cpp
    distributed)
