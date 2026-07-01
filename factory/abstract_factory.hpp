#pragma once
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "signals/signal_concepts.hpp"
#include "actors/actor_base.hpp"
#include "factory/ioc_container.hpp"
#include "scheduler/thread_pool.hpp"

struct ElaboratedDesign {
    std::vector<SignalBase*> signals;
    std::vector<ActorBase*> actors;
    std::unordered_map<SignalBase*, std::vector<ActorBase*>> sensitivity_map;
};

class AbstractFactory {
public:
    explicit AbstractFactory(IoCContainer& container, ThreadPool& pool)
        : container_(container), pool_(pool) {}

    ElaboratedDesign elaborate() {
        ElaboratedDesign design;

        for (auto& [name, sig] : container_.signals())
            design.signals.push_back(sig.get());

        for (auto& [name, act] : container_.actors())
            design.actors.push_back(act.get());

        for (const SignalBinding& binding : container_.sensitivity_bindings()) {
            SignalBase* sig = container_.get_signal(binding.signal_name);
            ActorBase* act = container_.get_actor(binding.actor_name);
            design.sensitivity_map[sig].push_back(act);
        }

        validate(design);
        return design;
    }

private:
    static void validate(const ElaboratedDesign& design) {
        if (design.actors.empty())
            throw std::runtime_error("Elaborated design contains no actors");
        if (design.signals.empty())
            throw std::runtime_error("Elaborated design contains no signals");
    }

    IoCContainer& container_;
    [[maybe_unused]] ThreadPool& pool_;
};
