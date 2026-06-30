module;
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

export module scheduler.simulation_cycle;

export import actors.base;
export import scheduler.delta_cycle;
export import scheduler.thread_pool;

export struct SimulationStats {
    uint64_t simulation_cycles{0};
    uint64_t total_delta_iterations{0};
    uint64_t total_actor_activations{0};
};

export using StimulusFn = std::function<bool(uint64_t simulation_time)>;

export class SimulationCycleEngine {
public:
    explicit SimulationCycleEngine(ThreadPool& pool)
        : delta_engine_(pool) {}

    void set_sensitivity_map(
        std::unordered_map<SignalBase*, std::vector<ActorBase*>> map)
    {
        sensitivity_map_ = std::move(map);
    }

    SimulationStats run(
        std::vector<ActorBase*> all_actors,
        std::vector<SignalBase*> all_signals,
        StimulusFn has_more_stimuli)
    {
        SimulationStats stats;
        ActorContext ctx;

        // Each simulation cycle re-runs every process once, then resolves all
        // signal transactions to a fixed point via delta cycles (which commit
        // scheduled -> current at every delta boundary). External stimuli for
        // the next cycle are applied by has_more_stimuli() as scheduled writes;
        // they surface as transactions at the start of the next cycle's delta
        // resolution and are committed there.
        do {
            DeltaCycleStats dc = delta_engine_.run(
                all_actors, all_signals, sensitivity_map_, ctx);

            stats.total_delta_iterations += dc.iterations;
            stats.total_actor_activations += dc.total_actor_activations;

            ++stats.simulation_cycles;
            ++ctx.simulation_time;
        } while (has_more_stimuli(ctx.simulation_time));

        return stats;
    }

private:
    DeltaCycleEngine delta_engine_;
    std::unordered_map<SignalBase*, std::vector<ActorBase*>> sensitivity_map_;
};
