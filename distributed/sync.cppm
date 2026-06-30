module;
#include <cstdint>

export module distributed.sync;

// Placeholder — global delta cycle barrier for distributed execution.
export class DistributedBarrier {
public:
    virtual ~DistributedBarrier() = default;
    virtual void wait(uint32_t delta_cycle) = 0;
    virtual void signal_done(uint32_t delta_cycle) = 0;
};
