#include <gtest/gtest.h>
#include <memory>
#include <stdexcept>
#include <string>

#include "signals/signal_bool.hpp"
#include "actors/actor_base.hpp"
#include "factory/ioc_container.hpp"

class DummyActor : public ActorBase {
public:
    explicit DummyActor(std::string n) : name_(std::move(n)) {}
    void execute(ActorContext&) override {}
    const std::string& name() const override { return name_; }
private:
    std::string name_;
};

TEST(IoCContainer, RegisterAndRetrieveSignal) {
    IoCContainer c;
    auto sig = std::make_shared<Signal<bool>>("s", false);
    c.register_signal("s", sig);
    EXPECT_EQ(c.get_signal("s"), sig.get());
}

TEST(IoCContainer, RegisterAndRetrieveActor) {
    IoCContainer c;
    auto act = std::make_unique<DummyActor>("a");
    DummyActor* raw = act.get();
    c.register_actor("a", std::move(act));
    EXPECT_EQ(c.get_actor("a"), raw);
}

TEST(IoCContainer, DuplicateSignalThrows) {
    IoCContainer c;
    c.register_signal("s", std::make_shared<Signal<bool>>("s", false));
    EXPECT_THROW(c.register_signal("s", std::make_shared<Signal<bool>>("s", false)),
                 std::runtime_error);
}

TEST(IoCContainer, DuplicateActorThrows) {
    IoCContainer c;
    c.register_actor("a", std::make_unique<DummyActor>("a"));
    EXPECT_THROW(c.register_actor("a", std::make_unique<DummyActor>("a")),
                 std::runtime_error);
}

TEST(IoCContainer, UnknownSignalThrows) {
    IoCContainer c;
    EXPECT_THROW(c.get_signal("nonexistent"), std::runtime_error);
}

TEST(IoCContainer, UnknownActorThrows) {
    IoCContainer c;
    EXPECT_THROW(c.get_actor("nonexistent"), std::runtime_error);
}

TEST(IoCContainer, SensitivityBindingsAreRecorded) {
    IoCContainer c;
    c.register_signal("s", std::make_shared<Signal<bool>>("s", false));
    c.register_actor("a", std::make_unique<DummyActor>("a"));
    c.bind_sensitivity("a", "s");
    ASSERT_EQ(c.sensitivity_bindings().size(), 1u);
    EXPECT_EQ(c.sensitivity_bindings()[0].actor_name, "a");
    EXPECT_EQ(c.sensitivity_bindings()[0].signal_name, "s");
}
