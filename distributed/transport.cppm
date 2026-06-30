module;
#include <string>

export module distributed.transport;

// Placeholder — network transport layer for inter-node signal replication.
export class Transport {
public:
    virtual ~Transport() = default;
    virtual void send(const std::string& destination, const std::string& payload) = 0;
    virtual std::string receive() = 0;
};
