module;
#include <cassert>
#include <string>
#include <utility>

export module signals.base;

export import signals.concepts;

export template <SignalValue T>
class Signal : public SignalBase {
public:
    explicit Signal(std::string signal_name, T initial = T{})
        : name_(std::move(signal_name))
        , current_value_(initial)
        , scheduled_value_(initial)
        , has_transaction_(false)
    {}

    T read_current() const { return current_value_; }

    void write_scheduled(T value) { scheduled_value_ = std::move(value); }

    void compute_transaction() override {
        has_transaction_ = !(scheduled_value_ == current_value_);
    }

    void commit_scheduled() override {
        assert(has_transaction_ && "commit_scheduled called with no pending transaction");
        current_value_ = scheduled_value_;
        has_transaction_ = false;
    }

    bool has_transaction() const override { return has_transaction_; }

    const std::string& name() const override { return name_; }

private:
    std::string name_;
    T current_value_;
    T scheduled_value_;
    bool has_transaction_;
};
