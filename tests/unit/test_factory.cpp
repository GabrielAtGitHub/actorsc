#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>

import signals.signal_bool;
import actors.base;
import factory.ioc_container;
import factory.abstract_factory;
import scheduler.thread_pool;

class DummyActor : public ActorBase {
public:
    explicit DummyActor(std::string n, Signal<bool>* s)
        : name_(std::move(n))
    { add_sensitivity(s); }
    void execute(ActorContext&) override {}
    const std::string& name() const override { return name_; }
private:
    std::string name_;
};

TEST(AbstractFactory, ElaborateProducesNonEmptyDesign) {
    IoCContainer c;
    auto sig = std::make_shared<Signal<bool>>("s", false);
    Signal<bool>* raw_sig = sig.get();
    c.register_signal("s", sig);
    c.register_actor("a", std::make_unique<DummyActor>("a", raw_sig));
    c.bind_sensitivity("a", "s");

    ThreadPool pool(1);
    AbstractFactory f(c, pool);
    ElaboratedDesign d = f.elaborate();
    EXPECT_FALSE(d.actors.empty());
    EXPECT_FALSE(d.signals.empty());
}

TEST(AbstractFactory, SensitivityMapIsBuilt) {
    IoCContainer c;
    auto sig = std::make_shared<Signal<bool>>("s", false);
    Signal<bool>* raw_sig = sig.get();
    c.register_signal("s", sig);
    auto act = std::make_unique<DummyActor>("a", raw_sig);
    ActorBase* raw_act = act.get();
    c.register_actor("a", std::move(act));
    c.bind_sensitivity("a", "s");

    ThreadPool pool(1);
    AbstractFactory f(c, pool);
    ElaboratedDesign d = f.elaborate();

    auto it = d.sensitivity_map.find(raw_sig);
    ASSERT_NE(it, d.sensitivity_map.end());
    ASSERT_EQ(it->second.size(), 1u);
    EXPECT_EQ(it->second[0], raw_act);
}

TEST(AbstractFactory, EmptyContainerThrows) {
    IoCContainer c;
    ThreadPool pool(1);
    AbstractFactory f(c, pool);
    EXPECT_THROW(f.elaborate(), std::runtime_error);
}
