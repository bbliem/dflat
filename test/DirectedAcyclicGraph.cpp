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
#include <memory>

#include "DirectedAcyclicGraph.h"

class IntDag;
typedef std::shared_ptr<IntDag> IntDagPtr;

class IntDag : public DirectedAcyclicGraph<int, std::set<IntDagPtr>>
{
public:
	IntDag(int i = 0)
		: DirectedAcyclicGraph(std::move(i))
	{
	}
};

TEST(DirectedAcyclicGraph, AllowsDiamond)
{
	IntDag diamond;
	IntDagPtr left(new IntDag(1));
	IntDagPtr right(new IntDag(2));
	IntDagPtr leaf(new IntDag(3));
	left->addChild(leaf);
	right->addChild(std::move(leaf));
	diamond.addChild(std::move(left));
	diamond.addChild(std::move(right));

	ASSERT_EQ(2, diamond.getChildren().size());
	const IntDagPtr& child1 = *diamond.getChildren().begin();
	const IntDagPtr& child2 = *++diamond.getChildren().begin();
	ASSERT_EQ(1, child1->getChildren().size());
	ASSERT_EQ(1, child2->getChildren().size());
	EXPECT_EQ(*child1->getChildren().begin(), *child2->getChildren().begin());
}

TEST(DirectedAcyclicGraph, AddChildAddsChild)
{
	IntDag dag;
	EXPECT_EQ(0, dag.getChildren().size());
	// r-value version
	dag.addChild(IntDagPtr(new IntDag));
	EXPECT_EQ(1, dag.getChildren().size());
	// const reference version
	IntDagPtr child(new IntDag);
	dag.addChild(child);
	EXPECT_EQ(2, dag.getChildren().size());
}
