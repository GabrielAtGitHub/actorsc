module;
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

export module factory.ioc_container;

export import actors.base;

export struct SignalBinding {
    std::string actor_name;
    std::string signal_name;
};

export class IoCContainer {
public:
    void register_signal(std::string signal_name,
                         std::shared_ptr<SignalBase> signal)
    {
        if (signals_.count(signal_name))
            throw std::runtime_error("Signal already registered: " + signal_name);
        signals_.emplace(std::move(signal_name), std::move(signal));
    }

    void register_actor(std::string actor_name,
                        std::unique_ptr<ActorBase> actor)
    {
        if (actors_.count(actor_name))
            throw std::runtime_error("Actor already registered: " + actor_name);
        actors_.emplace(std::move(actor_name), std::move(actor));
    }

    void bind_sensitivity(std::string actor_name, std::string signal_name) {
        sensitivity_bindings_.push_back({std::move(actor_name), std::move(signal_name)});
    }

    SignalBase* get_signal(const std::string& signal_name) const {
        auto it = signals_.find(signal_name);
        if (it == signals_.end())
            throw std::runtime_error("Unknown signal: " + signal_name);
        return it->second.get();
    }

    ActorBase* get_actor(const std::string& actor_name) const {
        auto it = actors_.find(actor_name);
        if (it == actors_.end())
            throw std::runtime_error("Unknown actor: " + actor_name);
        return it->second.get();
    }

    const std::unordered_map<std::string, std::shared_ptr<SignalBase>>& signals() const {
        return signals_;
    }

    const std::unordered_map<std::string, std::unique_ptr<ActorBase>>& actors() const {
        return actors_;
    }

    const std::vector<SignalBinding>& sensitivity_bindings() const {
        return sensitivity_bindings_;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<SignalBase>> signals_;
    std::unordered_map<std::string, std::unique_ptr<ActorBase>> actors_;
    std::vector<SignalBinding> sensitivity_bindings_;
};
