#include <gtest/gtest.h>

#include "Hypergraph.h"

TEST(Hypergraph, EliminatesDuplicates)
{
	Hypergraph hg;
	EXPECT_EQ(0, hg.getVertices().size());
	EXPECT_EQ(0, hg.getEdges().size());

	Hypergraph::Vertex v1 = "v1", v2 = "v2";
	hg.addVertex(v1);
	EXPECT_EQ(1, hg.getVertices().size());
	hg.addVertex(v1);
	EXPECT_EQ(1, hg.getVertices().size());
	hg.addVertex(v2);
	EXPECT_EQ(2, hg.getVertices().size());
	hg.addVertex("v3");
	EXPECT_EQ(3, hg.getVertices().size());

	Hypergraph::Edge e = { v1, v2 };
	hg.addEdge(e);
	EXPECT_EQ(1, hg.getEdges().size());
	hg.addEdge({ v1, v2, "foobar" });
	EXPECT_EQ(2, hg.getEdges().size());
	hg.addEdge(e);
	EXPECT_EQ(2, hg.getEdges().size());
}
