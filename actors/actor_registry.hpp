#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "actors/actor_base.hpp"

class ActorRegistry {
public:
    using FactoryFn = std::function<std::unique_ptr<ActorBase>()>;

    void register_type(std::string type_name, FactoryFn factory) {
        if (registry_.count(type_name))
            throw std::runtime_error("Actor type already registered: " + type_name);
        registry_.emplace(std::move(type_name), std::move(factory));
    }

    std::unique_ptr<ActorBase> create(const std::string& type_name) const {
        auto it = registry_.find(type_name);
        if (it == registry_.end())
            throw std::runtime_error("Unknown actor type: " + type_name);
        return it->second();
    }

    bool has_type(const std::string& type_name) const {
        return registry_.count(type_name) > 0;
    }

private:
    std::unordered_map<std::string, FactoryFn> registry_;
};
