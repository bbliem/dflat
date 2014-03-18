/*{{{
Copyright 2012-2014, Bernhard Bliem
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

struct DirectedAcyclicGraphTest : public ::testing::Test
{
	IntDag diamond;
	IntDag* diamondLeft;
	IntDag* diamondRight;
	IntDag* diamondLeaf;

	DirectedAcyclicGraphTest()
	{
		IntDagPtr left(new IntDag(1));
		IntDagPtr right(new IntDag(2));
		IntDagPtr leaf(new IntDag(3));

		diamondLeft = left.get();
		diamondRight = right.get();
		diamondLeaf = leaf.get();

		left->addChild(leaf);
		right->addChild(std::move(leaf));
		diamond.addChild(std::move(left));
		diamond.addChild(std::move(right));

		ASSERT_EQ(2, diamond.getChildren().size());
		const IntDagPtr& child1 = *diamond.getChildren().begin();
		const IntDagPtr& child2 = *++diamond.getChildren().begin();
		ASSERT_EQ(1, child1->getParents().size());
		ASSERT_EQ(1, child2->getParents().size());
		ASSERT_EQ(&diamond, child1->getParents().front());
		ASSERT_EQ(&diamond, child2->getParents().front());
	}
};

TEST_F(DirectedAcyclicGraphTest, MoveConstructorUpdatesParentsOfChildren)
{
	IntDag::Children childrenBeforeMove = diamond.getChildren();
	IntDag parentMoved = std::move(diamond);
	EXPECT_EQ(childrenBeforeMove, parentMoved.getChildren());

	EXPECT_EQ(&parentMoved, diamondLeft->getParents().front());
	EXPECT_EQ(&parentMoved, diamondRight->getParents().front());
}

TEST_F(DirectedAcyclicGraphTest, AssignmentOperatorUpdatesParentsOfChildren)
{
	IntDag::Children childrenBeforeAssignment = diamond.getChildren();
	IntDag parentAssigned;
	parentAssigned = std::move(diamond);
	EXPECT_EQ(childrenBeforeAssignment, parentAssigned.getChildren());

	EXPECT_EQ(&parentAssigned, diamondLeft->getParents().front());
	EXPECT_EQ(&parentAssigned, diamondRight->getParents().front());
}

TEST_F(DirectedAcyclicGraphTest, AddChildAddsChild)
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

TEST_F(DirectedAcyclicGraphTest, AddChildAddsParent)
{
	IntDag dag;
	// r-value version
	dag.addChild(IntDagPtr(new IntDag));
	EXPECT_EQ(&dag, (*dag.getChildren().begin())->getParents().front());
	// const reference version
	IntDagPtr child(new IntDag);
	dag.addChild(child);
	EXPECT_EQ(&dag, child->getParents().front());
}
