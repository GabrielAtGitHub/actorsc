#pragma once
#include <string>

// Placeholder — network transport layer for inter-node signal replication.
class Transport {
public:
    virtual ~Transport() = default;
    virtual void send(const std::string& destination, const std::string& payload) = 0;
    virtual std::string receive() = 0;
};
