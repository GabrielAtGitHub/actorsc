module;
#include <cstdint>

export module actors.context;

export struct ActorContext {
    uint64_t simulation_time{0};
    uint32_t delta_cycle{0};
};
