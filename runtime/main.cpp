#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>

#include "signals/signal_base.hpp"
#include "actors/actor_base.hpp"
#include "factory/ioc_container.hpp"
#include "runtime/elaborated_design.hpp"

// ---------------------------------------------------------------------------
// Example actors: a NOT gate and a buffer, both operating on bool signals.
// ---------------------------------------------------------------------------

class NotGate : public ActorBase {
public:
    explicit NotGate(std::string actor_name, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(actor_name)), in_(in), out_(out)
    {
        add_sensitivity(in_);
    }

    void execute(ActorContext&) override {
        out_->write_scheduled(!in_->read_current());
    }

    const std::string& name() const override { return name_; }

private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

class Buffer : public ActorBase {
public:
    explicit Buffer(std::string actor_name, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(actor_name)), in_(in), out_(out)
    {
        add_sensitivity(in_);
    }

    void execute(ActorContext&) override {
        out_->write_scheduled(in_->read_current());
    }

    const std::string& name() const override { return name_; }

private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

// ---------------------------------------------------------------------------
// main: A=true --NOT--> B --BUF--> C
// ---------------------------------------------------------------------------

int main() {
    auto container = std::make_unique<IoCContainer>();

    auto sig_a = std::make_shared<Signal<bool>>("A", true);
    auto sig_b = std::make_shared<Signal<bool>>("B", false);
    auto sig_c = std::make_shared<Signal<bool>>("C", false);

    Signal<bool>* a = sig_a.get();
    Signal<bool>* b = sig_b.get();
    Signal<bool>* c = sig_c.get();

    container->register_signal("A", sig_a);
    container->register_signal("B", sig_b);
    container->register_signal("C", sig_c);

    container->register_actor("NOT_AB", std::make_unique<NotGate>("NOT_AB", a, b));
    container->register_actor("BUF_BC", std::make_unique<Buffer>("BUF_BC", b, c));

    container->bind_sensitivity("NOT_AB", "A");
    container->bind_sensitivity("BUF_BC", "B");

    uint64_t max_cycles = 3;

    Runtime rt(std::move(container));

    auto stimuli = [&](uint64_t sim_time) -> bool {
        if (sim_time == 1)
            a->write_scheduled(!a->read_current());
        return sim_time < max_cycles;
    };

    SimulationStats stats = rt.run(stimuli);

    std::printf("Simulation complete.\n");
    std::printf("  Simulation cycles  : %llu\n",
                static_cast<unsigned long long>(stats.simulation_cycles));
    std::printf("  Delta iterations   : %llu\n",
                static_cast<unsigned long long>(stats.total_delta_iterations));
    std::printf("  Actor activations  : %llu\n",
                static_cast<unsigned long long>(stats.total_actor_activations));
    std::printf("  Final A=%d  B=%d  C=%d\n",
                static_cast<int>(a->read_current()),
                static_cast<int>(b->read_current()),
                static_cast<int>(c->read_current()));

    return 0;
}
