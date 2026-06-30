module;
#include <stdexcept>
#include <unordered_map>
#include <vector>

export module factory.abstract_factory;

export import factory.ioc_container;
export import scheduler.thread_pool;

export struct ElaboratedDesign {
    std::vector<SignalBase*> signals;
    std::vector<ActorBase*> actors;
    std::unordered_map<SignalBase*, std::vector<ActorBase*>> sensitivity_map;
};

export class AbstractFactory {
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
