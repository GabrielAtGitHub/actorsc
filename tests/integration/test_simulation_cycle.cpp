#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

#include "signals/signal_bool.hpp"
#include "signals/signal_int.hpp"
#include "actors/actor_base.hpp"
#include "factory/ioc_container.hpp"
#include "runtime/elaborated_design.hpp"

class Accumulator : public ActorBase {
public:
    Accumulator(std::string n, Signal<int>* trigger, Signal<int>* accum)
        : name_(std::move(n)), trigger_(trigger), accum_(accum)
    { add_sensitivity(trigger_); }

    void execute(ActorContext&) override {
        accum_->write_scheduled(accum_->read_current() + trigger_->read_current());
    }

    const std::string& name() const override { return name_; }

private:
    std::string name_;
    Signal<int>* trigger_;
    Signal<int>* accum_;
};

TEST(SimulationCycle, RunsMultipleCycles) {
    auto c = std::make_unique<IoCContainer>();
    auto trig  = std::make_shared<Signal<int>>("trig",  1);
    auto accum = std::make_shared<Signal<int>>("accum", 0);
    Signal<int>* raw_trig = trig.get();

    c->register_signal("trig",  trig);
    c->register_signal("accum", accum);
    c->register_actor("ACC", std::make_unique<Accumulator>("ACC", trig.get(), accum.get()));
    c->bind_sensitivity("ACC", "trig");

    constexpr uint64_t MAX_CYCLES = 5;
    Runtime rt(std::move(c), 2);

    SimulationStats stats = rt.run([&](uint64_t t) -> bool {
        if (t < MAX_CYCLES)
            raw_trig->write_scheduled(static_cast<int>(t + 1));
        return t < MAX_CYCLES;
    });

    EXPECT_EQ(stats.simulation_cycles, MAX_CYCLES);
}

TEST(SimulationCycle, DeterministicOutputAcrossRuns) {
    auto make_rt = [] {
        auto c = std::make_unique<IoCContainer>();
        auto a = std::make_shared<Signal<bool>>("A", true);
        auto b = std::make_shared<Signal<bool>>("B", false);

        struct NotGate : ActorBase {
            NotGate(Signal<bool>* i, Signal<bool>* o) : in(i), out(o) { add_sensitivity(in); }
            void execute(ActorContext&) override { out->write_scheduled(!in->read_current()); }
            const std::string& name() const override { return n; }
            std::string n{"NOT"};
            Signal<bool>* in;
            Signal<bool>* out;
        };

        c->register_signal("A", a);
        c->register_signal("B", b);
        c->register_actor("NOT", std::make_unique<NotGate>(a.get(), b.get()));
        c->bind_sensitivity("NOT", "A");
        return std::make_unique<Runtime>(std::move(c), 1);
    };

    auto rt1 = make_rt();
    auto rt2 = make_rt();

    auto stim = [](uint64_t t) { return t < 3; };
    SimulationStats s1 = rt1->run(stim);
    SimulationStats s2 = rt2->run(stim);

    EXPECT_EQ(s1.simulation_cycles,        s2.simulation_cycles);
    EXPECT_EQ(s1.total_delta_iterations,   s2.total_delta_iterations);
}
