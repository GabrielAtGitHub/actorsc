#pragma once
#include <cstdint>
#include <future>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "actors/actor_base.hpp"
#include "actors/actor_context.hpp"
#include "signals/signal_concepts.hpp"
#include "scheduler/thread_pool.hpp"

struct DeltaCycleStats {
    uint32_t iterations{0};
    uint64_t total_actor_activations{0};
};

// Upper bound on delta iterations within a single simulation cycle. A
// well-formed (combinational, acyclic-enough) design converges to a fixed
// point well before this; hitting the limit means the actor graph oscillates
// and would otherwise hang, so we fail loudly instead.
inline constexpr uint32_t kMaxDeltaIterations = 100000;

class DeltaCycleEngine {
public:
    explicit DeltaCycleEngine(ThreadPool& pool) : pool_(pool) {}

    // Resolves all signal transactions to a fixed point.
    //
    // Each delta cycle:
    //   1. Active processes execute (read current, write scheduled). The
    //      current value of every signal is frozen during this phase, so the
    //      result is independent of execution order (determinism invariant).
    //   2. Transactions are recomputed (scheduled != current).
    //   3. Processes sensitive to a signal that has a transaction are queued
    //      for the *next* delta cycle.
    //   4. Signals with transactions are committed (scheduled -> current).
    //      This update at the delta-cycle boundary is what lets values
    //      propagate through the graph and the loop reach a fixed point.
    DeltaCycleStats run(
        std::vector<ActorBase*> active,
        const std::vector<SignalBase*>& all_signals,
        const std::unordered_map<SignalBase*, std::vector<ActorBase*>>& sensitivity_map,
        ActorContext& ctx)
    {
        DeltaCycleStats stats;

        while (!active.empty()) {
            if (stats.iterations >= kMaxDeltaIterations)
                throw std::runtime_error(
                    "delta cycle failed to converge: actor graph oscillates");

            ctx.delta_cycle = stats.iterations;
            stats.total_actor_activations += active.size();

            execute_concurrent(active, ctx);

            for (SignalBase* s : all_signals)
                s->compute_transaction();

            // Queue next-cycle actors *before* committing (commit clears the
            // transaction flag that collect_sensitive inspects).
            active = collect_sensitive(all_signals, sensitivity_map);

            for (SignalBase* s : all_signals)
                if (s->has_transaction())
                    s->commit_scheduled();

            ++stats.iterations;
        }

        return stats;
    }

private:
    void execute_concurrent(const std::vector<ActorBase*>& actors, ActorContext& ctx) {
        std::vector<std::future<void>> futs;
        futs.reserve(actors.size());
        for (ActorBase* a : actors)
            futs.push_back(pool_.submit([a, &ctx] { a->execute(ctx); }));
        for (auto& f : futs)
            f.get();
    }

    static std::vector<ActorBase*> collect_sensitive(
        const std::vector<SignalBase*>& signals,
        const std::unordered_map<SignalBase*, std::vector<ActorBase*>>& sensitivity_map)
    {
        std::unordered_set<ActorBase*> seen;
        std::vector<ActorBase*> result;

        for (SignalBase* sig : signals) {
            if (!sig->has_transaction())
                continue;
            auto it = sensitivity_map.find(sig);
            if (it == sensitivity_map.end())
                continue;
            for (ActorBase* a : it->second) {
                if (seen.insert(a).second)
                    result.push_back(a);
            }
        }
        return result;
    }

    ThreadPool& pool_;
};
