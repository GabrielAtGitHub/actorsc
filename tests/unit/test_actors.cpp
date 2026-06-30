#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <vector>

import signals.base;
import signals.signal_bool;
import actors.base;

class EchoActor : public ActorBase {
public:
    explicit EchoActor(std::string n, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(n)), in_(in), out_(out)
    {
        add_sensitivity(in_);
    }

    void execute(ActorContext& ctx) override {
        last_time_ = ctx.simulation_time;
        last_delta_ = ctx.delta_cycle;
        out_->write_scheduled(in_->read_current());
        executed_ = true;
    }

    const std::string& name() const override { return name_; }

    bool executed_{false};
    uint64_t last_time_{0};
    uint32_t last_delta_{0};

private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

TEST(ActorBase, SensitivityListContainsRegisteredSignal) {
    Signal<bool> in("in", false);
    Signal<bool> out("out", false);
    EchoActor a("echo", &in, &out);

    ASSERT_EQ(a.sensitivity_list().size(), 1u);
    EXPECT_EQ(a.sensitivity_list()[0], static_cast<SignalBase*>(&in));
}

TEST(ActorBase, ExecuteWritesScheduledFromCurrent) {
    Signal<bool> in("in", true);
    Signal<bool> out("out", false);
    EchoActor a("echo", &in, &out);

    ActorContext ctx;
    a.execute(ctx);

    out.compute_transaction();
    EXPECT_TRUE(out.has_transaction());
    out.commit_scheduled();
    EXPECT_EQ(out.read_current(), true);
}

TEST(ActorBase, ExecuteReceivesCorrectContext) {
    Signal<bool> in("in", false);
    Signal<bool> out("out", false);
    EchoActor a("echo", &in, &out);

    ActorContext ctx;
    ctx.simulation_time = 5;
    ctx.delta_cycle = 3;
    a.execute(ctx);

    EXPECT_EQ(a.last_time_, 5u);
    EXPECT_EQ(a.last_delta_, 3u);
}

TEST(ActorBase, NameIsPreserved) {
    Signal<bool> in("in", false);
    Signal<bool> out("out", false);
    EchoActor a("my_actor", &in, &out);
    EXPECT_EQ(a.name(), "my_actor");
}
