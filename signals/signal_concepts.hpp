#pragma once
#include <concepts>
#include <string>

template <typename T>
concept SignalValue = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
};

class SignalBase {
public:
    virtual ~SignalBase() = default;
    virtual void compute_transaction() = 0;
    virtual void commit_scheduled() = 0;
    virtual bool has_transaction() const = 0;
    virtual const std::string& name() const = 0;
};
