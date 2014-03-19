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

#include "ItemTree.h"

struct ItemTreeTest : public ::testing::Test
{
	ItemTreePtr undefNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr orNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};
	ItemTreePtr andNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}}}};

	ItemTreePtr cheapNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr expensiveNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};

	ItemTreeTest()
	{
		cheapNode->getNode()->setCost(3);
		expensiveNode->getNode()->setCost(4);
	}
};

TEST_F(ItemTreeTest, AddChildAndMergeWithoutMerging)
{
	// Item tree 1:
	// a
	// |- b
	// \- c
	ItemTreePtr t1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"a"}}}}};
	ItemTreePtr t1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"b"}}}}};
	ItemTreePtr t1_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"c"}}}}};
	t1->addChildAndMerge(std::move(t1_1));
	t1->addChildAndMerge(std::move(t1_2));

	// Item tree 2:
	// a
	// |- b
	// \- d
	ItemTreePtr t2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"a"}}}}};
	ItemTreePtr t2_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"b"}}}}};
	ItemTreePtr t2_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"d"}}}}};
	t2->addChildAndMerge(std::move(t2_1));
	t2->addChildAndMerge(std::move(t2_2));

	// Add them as children to some item tree
	EXPECT_EQ(0, undefNode->getChildren().size());
	EXPECT_NE(undefNode->getChildren().end(), undefNode->addChildAndMerge(std::move(t1)));
	EXPECT_EQ(1, undefNode->getChildren().size());
	EXPECT_NE(undefNode->getChildren().end(), undefNode->addChildAndMerge(std::move(t2)));
	EXPECT_EQ(2, undefNode->getChildren().size());
}

TEST_F(ItemTreeTest, AddChildAndMergeWithMerging)
{
	// Item tree 1:
	// a
	// |- b
	// \- c
	ItemTreePtr t1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"a"}}}}};
	ItemTreePtr t1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"b"}}}}};
	ItemTreePtr t1_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"c"}}}}};
	t1->addChildAndMerge(std::move(t1_1));
	t1->addChildAndMerge(std::move(t1_2));

	// Item tree 2:
	// a
	// |- b
	// \- c
	ItemTreePtr t2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"a"}}}}};
	ItemTreePtr t2_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{"b"}}}}};
	ItemTreePtr t2_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{"c"}}}}};
	t2->addChildAndMerge(std::move(t2_1));
	t2->addChildAndMerge(std::move(t2_2));

	// Add them as children to some item tree
	EXPECT_EQ(0, undefNode->getChildren().size());
	EXPECT_NE(undefNode->getChildren().end(), undefNode->addChildAndMerge(std::move(t1)));
	EXPECT_EQ(1, undefNode->getChildren().size());
	EXPECT_EQ(undefNode->getChildren().end(), undefNode->addChildAndMerge(std::move(t2)));
	EXPECT_EQ(1, undefNode->getChildren().size());
}

TEST_F(ItemTreeTest, AddChildAndMergeSetsParent)
{
	const ItemTree* childPtr = undefNode.get();
	EXPECT_EQ(nullptr, childPtr->getNode()->getParent());
	EXPECT_TRUE(childPtr->getParents().empty());

	orNode->addChildAndMerge(std::move(undefNode));
	EXPECT_EQ(orNode->getNode().get(), childPtr->getNode()->getParent());
	EXPECT_EQ(ItemTree::Parents{orNode.get()}, childPtr->getParents());
}

TEST_F(ItemTreeTest, AddChildAndMergeRetainsCheapChildOfOrNode)
{
	const auto expectedCost = cheapNode->getNode()->getCost();
	orNode->addChildAndMerge(std::move(cheapNode));
	orNode->addChildAndMerge(std::move(expensiveNode));
	ASSERT_EQ(1, orNode->getChildren().size());
	EXPECT_EQ(expectedCost, (*orNode->getChildren().begin())->getNode()->getCost());
}

TEST_F(ItemTreeTest, AddChildAndMergeReplacesExpensiveChildOfOrNode)
{
	const auto expectedCost = cheapNode->getNode()->getCost();
	orNode->addChildAndMerge(std::move(expensiveNode));
	orNode->addChildAndMerge(std::move(cheapNode));
	ASSERT_EQ(1, orNode->getChildren().size());
	EXPECT_EQ(expectedCost, (*orNode->getChildren().begin())->getNode()->getCost());
}

TEST_F(ItemTreeTest, AddChildAndMergeRetainsExpensiveChildOfAndNode)
{
	const auto expectedCost = expensiveNode->getNode()->getCost();
	andNode->addChildAndMerge(std::move(expensiveNode));
	andNode->addChildAndMerge(std::move(cheapNode));
	ASSERT_EQ(1, andNode->getChildren().size());
	EXPECT_EQ(expectedCost, (*andNode->getChildren().begin())->getNode()->getCost());
}

TEST_F(ItemTreeTest, AddChildAndMergeReplacesCheapChildOfAndNode)
{
	const auto expectedCost = expensiveNode->getNode()->getCost();
	andNode->addChildAndMerge(std::move(cheapNode));
	andNode->addChildAndMerge(std::move(expensiveNode));
	ASSERT_EQ(1, andNode->getChildren().size());
	EXPECT_EQ(expectedCost, (*andNode->getChildren().begin())->getNode()->getCost());
}
