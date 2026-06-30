module;
#include <string>
#include <utility>

export module distributed.node;

// Placeholder — distributed runtime is a future extension.
// See PROJECT_PLAN.md §5.2 for the planned interface.
export class DistributedNode {
public:
    explicit DistributedNode(std::string node_id) : node_id_(std::move(node_id)) {}
    const std::string& node_id() const { return node_id_; }
private:
    std::string node_id_;
};
