/*{{{
Copyright 2012-2016, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/
//}}}
#include <gtest/gtest.h>

#include "Hypergraph.h"

TEST(Hypergraph, EliminatesDuplicates)
{
	Hypergraph hg;
	EXPECT_EQ(0, hg.getVertices().size());
	EXPECT_EQ(0, hg.getEdges().size());

	Hypergraph::Vertex v1("v1");
	Hypergraph::Vertex v2("v2");
	hg.addVertex(v1);
	EXPECT_EQ(1, hg.getVertices().size());
	hg.addVertex(v1);
	EXPECT_EQ(1, hg.getVertices().size());
	hg.addVertex(v2);
	EXPECT_EQ(2, hg.getVertices().size());
	hg.addVertex({"v3"});
	EXPECT_EQ(3, hg.getVertices().size());

	Hypergraph::Edge e = { v1, v2 };
	hg.addEdge(e);
	EXPECT_EQ(1, hg.getEdges().size());
	hg.addEdge({ v1, v2, {"foobar"} });
	EXPECT_EQ(2, hg.getEdges().size());
	hg.addEdge(e);
	EXPECT_EQ(2, hg.getEdges().size());
}
