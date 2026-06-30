#include <gtest/gtest.h>
#include <memory>
#include <string>

import signals.base;
import signals.signal_bool;
import signals.signal_int;

TEST(SignalConcepts, BoolSatisfiesConcept) {
    static_assert(SignalValue<bool>);
}

TEST(SignalConcepts, IntSatisfiesConcept) {
    static_assert(SignalValue<int>);
}

TEST(SignalConcepts, StringSatisfiesConcept) {
    static_assert(SignalValue<std::string>);
}

TEST(SignalBool, InitialCurrentValueMatchesConstructor) {
    Signal<bool> s("x", true);
    EXPECT_EQ(s.read_current(), true);
}

TEST(SignalBool, NoTransactionAfterConstruction) {
    Signal<bool> s("x", false);
    s.compute_transaction();
    EXPECT_FALSE(s.has_transaction());
}

TEST(SignalBool, WriteScheduledDoesNotChangeCurrentDuringDeltaCycle) {
    Signal<bool> s("x", false);
    s.write_scheduled(true);
    EXPECT_EQ(s.read_current(), false);
}

TEST(SignalBool, ComputeTransactionDetectsDifference) {
    Signal<bool> s("x", false);
    s.write_scheduled(true);
    s.compute_transaction();
    EXPECT_TRUE(s.has_transaction());
}

TEST(SignalBool, CommitAppliesScheduledToCurrent) {
    Signal<bool> s("x", false);
    s.write_scheduled(true);
    s.compute_transaction();
    s.commit_scheduled();
    EXPECT_EQ(s.read_current(), true);
    EXPECT_FALSE(s.has_transaction());
}

TEST(SignalBool, NoTransactionWhenScheduledEqualsCurrentValue) {
    Signal<bool> s("x", true);
    s.write_scheduled(true);
    s.compute_transaction();
    EXPECT_FALSE(s.has_transaction());
}

TEST(SignalBool, NameIsPreserved) {
    Signal<bool> s("my_signal", false);
    EXPECT_EQ(s.name(), "my_signal");
}

TEST(SignalInt, TransactionOnChange) {
    Signal<int> s("counter", 0);
    s.write_scheduled(42);
    s.compute_transaction();
    EXPECT_TRUE(s.has_transaction());
    s.commit_scheduled();
    EXPECT_EQ(s.read_current(), 42);
}

TEST(SignalInt, NoTransactionWhenValueUnchanged) {
    Signal<int> s("counter", 7);
    s.write_scheduled(7);
    s.compute_transaction();
    EXPECT_FALSE(s.has_transaction());
}

TEST(SignalBase, PolymorphicCommit) {
    auto sig = std::make_unique<Signal<int>>("v", 0);
    SignalBase* base = sig.get();
    sig->write_scheduled(99);
    base->compute_transaction();
    EXPECT_TRUE(base->has_transaction());
    base->commit_scheduled();
    EXPECT_EQ(sig->read_current(), 99);
    EXPECT_FALSE(base->has_transaction());
}
