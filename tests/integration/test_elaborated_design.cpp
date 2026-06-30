#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>

import signals.signal_bool;
import actors.base;
import factory.ioc_container;
import runtime.elaborated_design;

class NotGate : public ActorBase {
public:
    NotGate(std::string n, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(n)), in_(in), out_(out)
    { add_sensitivity(in_); }
    void execute(ActorContext&) override { out_->write_scheduled(!in_->read_current()); }
    const std::string& name() const override { return name_; }
private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

TEST(ElaboratedDesign, RunsWithoutCrash) {
    auto c = std::make_unique<IoCContainer>();
    auto sa = std::make_shared<Signal<bool>>("A", true);
    auto sb = std::make_shared<Signal<bool>>("B", false);
    c->register_signal("A", sa);
    c->register_signal("B", sb);
    c->register_actor("NOT_AB", std::make_unique<NotGate>("NOT_AB", sa.get(), sb.get()));
    c->bind_sensitivity("NOT_AB", "A");

    Runtime rt(std::move(c), 2);
    SimulationStats stats = rt.run([](uint64_t t) { return t < 1; });
    EXPECT_GE(stats.simulation_cycles, 1u);
}

TEST(ElaboratedDesign, DesignContainsExpectedSignalsAndActors) {
    auto c = std::make_unique<IoCContainer>();
    auto sa = std::make_shared<Signal<bool>>("A", false);
    auto sb = std::make_shared<Signal<bool>>("B", false);
    c->register_signal("A", sa);
    c->register_signal("B", sb);
    c->register_actor("NOT_AB", std::make_unique<NotGate>("NOT_AB", sa.get(), sb.get()));
    c->bind_sensitivity("NOT_AB", "A");

    Runtime rt(std::move(c), 1);
    const ElaboratedDesign& d = rt.design();
    EXPECT_EQ(d.signals.size(), 2u);
    EXPECT_EQ(d.actors.size(), 1u);
}
