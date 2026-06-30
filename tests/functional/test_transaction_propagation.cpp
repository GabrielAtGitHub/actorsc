#include <gtest/gtest.h>
#include <string>
#include <unordered_map>
#include <vector>

import signals.signal_bool;
import signals.signal_int;
import actors.base;
import scheduler.thread_pool;
import scheduler.delta_cycle;

class PassThroughInt : public ActorBase {
public:
    PassThroughInt(std::string n, Signal<int>* in, Signal<int>* out)
        : name_(std::move(n)), in_(in), out_(out)
    { add_sensitivity(in_); }
    void execute(ActorContext&) override { out_->write_scheduled(in_->read_current()); }
    const std::string& name() const override { return name_; }
private:
    std::string name_;
    Signal<int>* in_;
    Signal<int>* out_;
};

TEST(TransactionPropagation, IntSignalPropagatesCorrectly) {
    Signal<int> src("src", 0);
    Signal<int> dst("dst", 0);
    PassThroughInt pt("pt", &src, &dst);

    ThreadPool pool(1);
    DeltaCycleEngine engine(pool);

    std::unordered_map<SignalBase*, std::vector<ActorBase*>> smap;
    smap[static_cast<SignalBase*>(&src)].push_back(&pt);

    std::vector<SignalBase*> signals = {
        static_cast<SignalBase*>(&src),
        static_cast<SignalBase*>(&dst)
    };
    ActorContext ctx;

    src.write_scheduled(42);
    src.compute_transaction();
    engine.run({&pt}, signals, smap, ctx);

    // src's transaction is committed, then propagated through pt into dst.
    EXPECT_EQ(src.read_current(), 42);
    EXPECT_EQ(dst.read_current(), 42);
    dst.compute_transaction();
    EXPECT_FALSE(dst.has_transaction());
}

TEST(TransactionPropagation, NoActivationWithoutTransaction) {
    Signal<bool> s("s", false);
    bool executed = false;

    struct Spy : ActorBase {
        Spy(Signal<bool>* sig, bool& flag) : flag_(flag) { add_sensitivity(sig); }
        void execute(ActorContext&) override { flag_ = true; }
        const std::string& name() const override { return n_; }
        std::string n_{"spy"};
        bool& flag_;
    };

    Spy spy(&s, executed);
    ThreadPool pool(1);
    DeltaCycleEngine engine(pool);

    std::unordered_map<SignalBase*, std::vector<ActorBase*>> smap;
    smap[static_cast<SignalBase*>(&s)].push_back(&spy);
    std::vector<SignalBase*> signals = {static_cast<SignalBase*>(&s)};
    ActorContext ctx;

    engine.run({}, signals, smap, ctx);
    EXPECT_FALSE(executed);
}
