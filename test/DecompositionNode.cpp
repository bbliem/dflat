#include <gtest/gtest.h>

#include "DecompositionNode.h"

TEST(DecompositionNode, ReturnsBag)
{
	Hypergraph::Vertices bag = {"a", "b"};
	EXPECT_EQ(bag, DecompositionNode(bag).getBag());
}

TEST(DecompositionNode, HasUniqueGlobalId)
{
	DecompositionNode n1 = {{}};
	DecompositionNode n2 = {{}};

	EXPECT_TRUE(n1.getGlobalId() != n2.getGlobalId())
		<< "Global IDs of n1 and n2 are not different: " << n1.getGlobalId() << " and " << n2.getGlobalId();
}
