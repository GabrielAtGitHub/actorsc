#include <gtest/gtest.h>
#include <string>

#include "distributed/node.hpp"
#include "distributed/transport.hpp"
#include "distributed/sync.hpp"

// ---------------------------------------------------------------------------
// Distributed runtime is a FUTURE extension (PROJECT_PLAN.md §5.2, §7.4).
// Per CLAUDE.md §9.4 these tests are scaffolded but intentionally NOT
// implemented. They are registered as DISABLED so they neither run nor fail
// CI, but the placeholder interfaces are still compiled and kept honest.
// ---------------------------------------------------------------------------

TEST(DistributedScaffold, NodeHoldsItsId) {
    DistributedNode node("node-0");
    EXPECT_EQ(node.node_id(), "node-0");
}

TEST(DistributedScaffold, DISABLED_MultiNodeActorGraph) {
    GTEST_SKIP() << "distributed runtime not yet implemented";
}

TEST(DistributedScaffold, DISABLED_NetworkPartitionTolerance) {
    GTEST_SKIP() << "distributed runtime not yet implemented";
}

TEST(DistributedScaffold, DISABLED_GlobalDeltaCycleSynchronization) {
    GTEST_SKIP() << "distributed runtime not yet implemented";
}
