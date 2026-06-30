#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <vector>

import signals.signal_bool;
import actors.base;
import scheduler.thread_pool;
import scheduler.delta_cycle;

class NotActor : public ActorBase {
public:
    NotActor(std::string n, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(n)), in_(in), out_(out)
    { add_sensitivity(in_); }
    void execute(ActorContext&) override { out_->write_scheduled(!in_->read_current()); }
    const std::string& name() const override { return name_; }
private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

class BufActor : public ActorBase {
public:
    BufActor(std::string n, Signal<bool>* in, Signal<bool>* out)
        : name_(std::move(n)), in_(in), out_(out)
    { add_sensitivity(in_); }
    void execute(ActorContext&) override { out_->write_scheduled(in_->read_current()); }
    const std::string& name() const override { return name_; }
private:
    std::string name_;
    Signal<bool>* in_;
    Signal<bool>* out_;
};

// A=true --NOT--> B(false): NOT writes !true=false == B.current, so there is
// no transaction and the engine converges in a single delta cycle.
TEST(DeltaCycle, SingleActorConvergesInOneIteration) {
    Signal<bool> a("A", true);
    Signal<bool> b("B", false);
    NotActor not_ab("NOT_AB", &a, &b);

    ThreadPool pool(2);
    DeltaCycleEngine engine(pool);

    std::unordered_map<SignalBase*, std::vector<ActorBase*>> smap;
    smap[&a].push_back(&not_ab);

    std::vector<SignalBase*> signals = {&a, &b};
    ActorContext ctx;

    DeltaCycleStats stats = engine.run({&not_ab}, signals, smap, ctx);

    EXPECT_EQ(stats.iterations, 1u);
    EXPECT_EQ(b.read_current(), false);
}

// A=false --NOT--> B --BUF--> C, all starting false. The transaction on B
// must propagate through a second delta cycle to reach C.
TEST(DeltaCycle, PropagationAcrossTwoActors) {
    Signal<bool> a("A", false);
    Signal<bool> b("B", false);
    Signal<bool> c("C", false);

    NotActor not_ab("NOT_AB", &a, &b);
    BufActor buf_bc("BUF_BC", &b, &c);

    ThreadPool pool(2);
    DeltaCycleEngine engine(pool);

    std::unordered_map<SignalBase*, std::vector<ActorBase*>> smap;
    smap[static_cast<SignalBase*>(&a)].push_back(&not_ab);
    smap[static_cast<SignalBase*>(&b)].push_back(&buf_bc);

    std::vector<SignalBase*> signals = {
        static_cast<SignalBase*>(&a),
        static_cast<SignalBase*>(&b),
        static_cast<SignalBase*>(&c)
    };
    ActorContext ctx;

    DeltaCycleStats stats = engine.run({&not_ab}, signals, smap, ctx);

    EXPECT_EQ(stats.iterations, 2u);
    EXPECT_EQ(b.read_current(), true);   // !A
    EXPECT_EQ(c.read_current(), true);   // buffered B
}

// A=false --NOT--> B --NOT--> C. B becomes !A=true, C becomes !B=false.
// The system reaches a stable fixed point (no oscillation).
TEST(DeltaCycle, StabilisesAtFixedPoint) {
    Signal<bool> a("A", false);
    Signal<bool> b("B", false);
    Signal<bool> c("C", false);

    NotActor not1("NOT1", &a, &b);
    NotActor not2("NOT2", &b, &c);

    ThreadPool pool(2);
    DeltaCycleEngine engine(pool);

    std::unordered_map<SignalBase*, std::vector<ActorBase*>> smap;
    smap[static_cast<SignalBase*>(&a)].push_back(&not1);
    smap[static_cast<SignalBase*>(&b)].push_back(&not2);

    std::vector<SignalBase*> signals = {
        static_cast<SignalBase*>(&a),
        static_cast<SignalBase*>(&b),
        static_cast<SignalBase*>(&c)
    };
    ActorContext ctx;

    DeltaCycleStats stats = engine.run({&not1}, signals, smap, ctx);

    EXPECT_EQ(stats.iterations, 2u);
    EXPECT_EQ(b.read_current(), true);    // !A
    EXPECT_EQ(c.read_current(), false);   // !B
}
