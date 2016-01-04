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

#include "Application.h"
#include "UncompressedItemTree.h"

TEST(UncompressedItemTreeTest, AddBranch)
{
	UncompressedItemTree tree(ItemTree::Node{new ItemTreeNode});
	EXPECT_EQ(0, tree.getChildren().size());
	UncompressedItemTree::Branch branch = { ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::REJECT}} };
	tree.addBranch(branch.begin(), branch.end());
	EXPECT_EQ(1, tree.getChildren().size());
}

TEST(UncompressedItemTreeTest, CompressPropagatesCostNonRejecting)
{
	UncompressedItemTree tree(ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}});
	ItemTree::Node undefNode{new ItemTreeNode{{{"b"}}, {}, {{}}}};
	undefNode->setCost(4);
	ItemTree::Node rejectNode{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::REJECT}};
	// rejectNode should have ("almost") infinite cost
	const auto rejectCost = rejectNode->getCost();
	ASSERT_GT(rejectCost, undefNode->getCost());
	UncompressedItemTree::Branch branch1 = { undefNode };
	tree.addBranch(branch1.begin(), branch1.end());
	UncompressedItemTree::Branch branch2 = { rejectNode };
	tree.addBranch(branch2.begin(), branch2.end());
	ASSERT_EQ(2, tree.getChildren().size());
	EXPECT_NE(4, tree.getNode()->getCost());
	EXPECT_NE(rejectCost, tree.getNode()->getCost());
	ItemTreePtr result = tree.compress(false);
	EXPECT_EQ(4, result->getNode()->getCost());
}

TEST(UncompressedItemTreeTest, CompressPropagatesCostDefinedNonRejecting)
{
	UncompressedItemTree tree(ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}});
	ItemTree::Node cheapNode{new ItemTreeNode{{{"a"}}, {}, {{}}}};
	cheapNode->setCost(3);
	ItemTree::Node expensiveNode{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::ACCEPT}};
	expensiveNode->setCost(4);
	UncompressedItemTree::Branch branch1 = { cheapNode };
	tree.addBranch(branch1.begin(), branch1.end());
	UncompressedItemTree::Branch branch2 = { expensiveNode };
	tree.addBranch(branch2.begin(), branch2.end());
	ASSERT_EQ(2, tree.getChildren().size());
	EXPECT_NE(3, tree.getNode()->getCost());
	EXPECT_NE(4, tree.getNode()->getCost());
	ItemTreePtr result = tree.compress(true);
	EXPECT_EQ(4, result->getNode()->getCost());
}
