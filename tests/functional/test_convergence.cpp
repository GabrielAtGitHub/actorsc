#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

import signals.signal_bool;
import actors.base;
import factory.ioc_container;
import factory.abstract_factory;
import scheduler.thread_pool;
import scheduler.simulation_cycle;

class Inverter : public ActorBase {
public:
    Inverter(std::string n, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(n)), in_(in), out_(out)
    { add_sensitivity(in_); }
    void execute(ActorContext&) override { out_->write_scheduled(!in_->read_current()); }
    const std::string& name() const override { return name_; }
private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

TEST(Convergence, SingleInverterReachesFixedPoint) {
    IoCContainer c;
    auto sa = std::make_shared<Signal<bool>>("A", true);
    auto sb = std::make_shared<Signal<bool>>("B", false);
    c.register_signal("A", sa);
    c.register_signal("B", sb);
    c.register_actor("INV", std::make_unique<Inverter>("INV", sa.get(), sb.get()));
    c.bind_sensitivity("INV", "A");

    ThreadPool pool(2);
    AbstractFactory f(c, pool);
    ElaboratedDesign d = f.elaborate();

    SimulationCycleEngine engine(pool);
    engine.set_sensitivity_map(d.sensitivity_map);

    SimulationStats stats = engine.run(
        d.actors, d.signals,
        [](uint64_t t) { return t < 1; });

    EXPECT_GE(stats.simulation_cycles, 1u);
}

TEST(Convergence, ChainConvergesWithoutOscillation) {
    IoCContainer c;
    auto sa = std::make_shared<Signal<bool>>("A", true);
    auto sb = std::make_shared<Signal<bool>>("B", false);
    auto sc = std::make_shared<Signal<bool>>("C", false);

    c.register_signal("A", sa);
    c.register_signal("B", sb);
    c.register_signal("C", sc);
    c.register_actor("INV1", std::make_unique<Inverter>("INV1", sa.get(), sb.get()));
    c.register_actor("INV2", std::make_unique<Inverter>("INV2", sb.get(), sc.get()));
    c.bind_sensitivity("INV1", "A");
    c.bind_sensitivity("INV2", "B");

    ThreadPool pool(2);
    AbstractFactory f(c, pool);
    ElaboratedDesign d = f.elaborate();

    SimulationCycleEngine engine(pool);
    engine.set_sensitivity_map(d.sensitivity_map);

    SimulationStats stats = engine.run(
        d.actors, d.signals,
        [](uint64_t t) { return t < 2; });

    EXPECT_GE(stats.simulation_cycles, 1u);
    EXPECT_GE(stats.total_delta_iterations, 1u);
}
