module;
#include <string>
#include <vector>

export module actors.base;

export import signals.concepts;
export import actors.context;

export class ActorBase {
public:
    virtual ~ActorBase() = default;

    virtual void execute(ActorContext& ctx) = 0;

    virtual const std::string& name() const = 0;

    const std::vector<SignalBase*>& sensitivity_list() const {
        return sensitivity_list_;
    }

protected:
    void add_sensitivity(SignalBase* sig) {
        sensitivity_list_.push_back(sig);
    }

    std::vector<SignalBase*> sensitivity_list_;
};
