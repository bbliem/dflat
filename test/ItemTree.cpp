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
#include "ItemTree.h"

struct ItemTreeTest : public ::testing::Test
{
	ItemTreePtr undefNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr undefNode2{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr orNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};
	ItemTreePtr andNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}}}};
	ItemTreePtr acceptNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
	ItemTreePtr rejectNode{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};

	ItemTreePtr cheapNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr cheapNode2{new ItemTree{ItemTree::Node{new ItemTreeNode}}};
	ItemTreePtr expensiveNode{new ItemTree{ItemTree::Node{new ItemTreeNode}}};

	Application app{"test"};

	// Item tree 1: (undefined types)
	// a
	// |- b
	// \- c
	ItemTreePtr t1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}}}}};

	// Item tree 2: Same as 1.
	ItemTreePtr t2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}}}}};

	// Item tree 3: (undefined types)
	// a
	// |- b
	// \- d
	ItemTreePtr t3{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}}}}};

	// Item tree 4: (empty item sets)
	// or
	// |- undef
	// \- reject
	ItemTreePtr t4{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 5: (empty item sets)
	// or
	// |- undef
	// |  \- accept
	// \- reject
	ItemTreePtr t5{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 6: (empty item sets)
	// or
	// \- undef
	ItemTreePtr t6{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 7: (empty item sets)
	// or
	// |- and
	// |  |- accept
	// |  |- undef
	// |  \- reject
	// \- undef
	ItemTreePtr t7{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 8: (empty item sets)
	// or
	// \- undef
	//    |- reject
	//    \- undef
	ItemTreePtr t8{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 9: (empty item sets)
	// and
	// |- undef
	// |- reject
	// \- accept
	ItemTreePtr t9{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}}}};

	// Item tree 10: (empty item sets)
	// or
	// |- undef
	// |- reject
	// \- accept
	ItemTreePtr t10{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 11: (empty item sets)
	// and
	// |- undef
	// \- accept
	ItemTreePtr t11{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}}}};

	// Item tree 12: (empty item sets)
	// or
	// |- undef
	// \- reject
	ItemTreePtr t12{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 13
	// a or
	// |- undef b 2
	// \- undef c 3
	ItemTreePtr t13{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 14: Same as 13.
	ItemTreePtr t14{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 15
	// a or
	// |- undef b 1
	// \- undef c 2
	ItemTreePtr t15{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 16
	// a or
	// |- undef b 1
	// \- undef c 3
	ItemTreePtr t16{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 17
	// a or
	// |- undef b 2
	// \- undef c 1
	ItemTreePtr t17{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	// Item tree 18
	// a or
	// \- undef b 0
	ItemTreePtr t18{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"a"}}, {}, {{}}, ItemTreeNode::Type::OR}}}};

	ItemTreeTest()
	{
		cheapNode->getNode()->setCost(3);
		cheapNode2->getNode()->setCost(3);
		expensiveNode->getNode()->setCost(4);

		ItemTreePtr t1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}}}}};
		ItemTreePtr t1_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}}}}};
		t1->addChildAndMerge(std::move(t1_1));
		t1->addChildAndMerge(std::move(t1_2));

		ItemTreePtr t2_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}}}}};
		ItemTreePtr t2_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}}}}};
		t2->addChildAndMerge(std::move(t2_1));
		t2->addChildAndMerge(std::move(t2_2));

		ItemTreePtr t3_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}}}}};
		ItemTreePtr t3_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"d"}}}}}};
		t3->addChildAndMerge(std::move(t3_1));
		t3->addChildAndMerge(std::move(t3_2));

		ItemTreePtr t4_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t4_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		t4->addChildAndMerge(std::move(t4_1));
		t4->addChildAndMerge(std::move(t4_2));

		ItemTreePtr t5_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t5_1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
		ItemTreePtr t5_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		t5_1->addChildAndMerge(std::move(t5_1_1));
		t5->addChildAndMerge(std::move(t5_1));
		t5->addChildAndMerge(std::move(t5_2));

		ItemTreePtr t6_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t6->addChildAndMerge(std::move(t6_1));

		ItemTreePtr t7_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}}}};
		ItemTreePtr t7_1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
		ItemTreePtr t7_1_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t7_1_3{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		ItemTreePtr t7_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t7_1->addChildAndMerge(std::move(t7_1_1));
		t7_1->addChildAndMerge(std::move(t7_1_2));
		t7_1->addChildAndMerge(std::move(t7_1_3));
		t7->addChildAndMerge(std::move(t7_1));
		t7->addChildAndMerge(std::move(t7_2));

		ItemTreePtr t8_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t8_1_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		ItemTreePtr t8_1_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t8_1->addChildAndMerge(std::move(t8_1_1));
		t8_1->addChildAndMerge(std::move(t8_1_2));
		t8->addChildAndMerge(std::move(t8_1));

		ItemTreePtr t9_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t9_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		ItemTreePtr t9_3{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
		t9->addChildAndMerge(std::move(t9_1));
		t9->addChildAndMerge(std::move(t9_2));
		t9->addChildAndMerge(std::move(t9_3));

		ItemTreePtr t10_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t10_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		ItemTreePtr t10_3{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
		t10->addChildAndMerge(std::move(t10_1));
		t10->addChildAndMerge(std::move(t10_2));
		t10->addChildAndMerge(std::move(t10_3));

		ItemTreePtr t11_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t11_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}}}};
		t11->addChildAndMerge(std::move(t11_1));
		t11->addChildAndMerge(std::move(t11_2));

		ItemTreePtr t12_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t12_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}}}};
		t12->addChildAndMerge(std::move(t12_1));
		t12->addChildAndMerge(std::move(t12_2));

		ItemTreePtr t13_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t13_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t13_1->getNode()->setCost(2);
		t13_2->getNode()->setCost(3);
		t13->addChildAndMerge(std::move(t13_1));
		t13->addChildAndMerge(std::move(t13_2));

		ItemTreePtr t14_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t14_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t14_1->getNode()->setCost(2);
		t14_2->getNode()->setCost(3);
		t14->addChildAndMerge(std::move(t14_1));
		t14->addChildAndMerge(std::move(t14_2));

		ItemTreePtr t15_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t15_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t15_1->getNode()->setCost(1);
		t15_2->getNode()->setCost(2);
		t15->addChildAndMerge(std::move(t15_1));
		t15->addChildAndMerge(std::move(t15_2));

		ItemTreePtr t16_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t16_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t16_1->getNode()->setCost(1);
		t16_2->getNode()->setCost(3);
		t16->addChildAndMerge(std::move(t16_1));
		t16->addChildAndMerge(std::move(t16_2));

		ItemTreePtr t17_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		ItemTreePtr t17_2{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"c"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t17_1->getNode()->setCost(2);
		t17_2->getNode()->setCost(1);
		t17->addChildAndMerge(std::move(t17_1));
		t17->addChildAndMerge(std::move(t17_2));

		ItemTreePtr t18_1{new ItemTree{ItemTree::Node{new ItemTreeNode{{{"b"}}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}}}};
		t18_1->getNode()->setCost(0);
		t18->addChildAndMerge(std::move(t18_1));
	}
};

TEST_F(ItemTreeTest, AddChildAndMergeWithoutMerging)
{
	// Add t1 and t3 as children to some item tree
	EXPECT_EQ(0, undefNode->getChildren().size());
	undefNode->addChildAndMerge(std::move(t1));
	EXPECT_EQ(1, undefNode->getChildren().size());
	undefNode->addChildAndMerge(std::move(t3));
	EXPECT_EQ(2, undefNode->getChildren().size());
}

TEST_F(ItemTreeTest, AddChildAndMergeWithMerging)
{
	// Add t1 and t2 as children to some item tree
	EXPECT_EQ(0, undefNode->getChildren().size());
	undefNode->addChildAndMerge(std::move(t1));
	EXPECT_EQ(1, undefNode->getChildren().size());
	undefNode->addChildAndMerge(std::move(t2));
	EXPECT_EQ(1, undefNode->getChildren().size());
}

TEST_F(ItemTreeTest, AddChildAndMergeSetsParent)
{
	const ItemTree* childPtr = undefNode.get();
	EXPECT_EQ(nullptr, childPtr->getNode()->getParent());
	orNode->addChildAndMerge(std::move(undefNode));
	EXPECT_EQ(orNode->getNode().get(), childPtr->getNode()->getParent());
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

TEST_F(ItemTreeTest, AddChildAndMergeMergesAfterReplacing)
{
	orNode->addChildAndMerge(std::move(expensiveNode));
	orNode->addChildAndMerge(std::move(cheapNode));
	EXPECT_EQ(1, orNode->getChildren().size());
	orNode->addChildAndMerge(std::move(cheapNode2));
	ASSERT_EQ(1, orNode->getChildren().size());
	EXPECT_EQ(2, (*orNode->getChildren().begin())->getNode()->getExtensionPointers().size());
}

TEST_F(ItemTreeTest, PruneUndefinedLeaf)
{
	t4->pruneUndefined();
	ASSERT_EQ(1, t4->getChildren().size());
	EXPECT_EQ(ItemTreeNode::Type::REJECT, (*t4->getChildren().begin())->getNode()->getType());
}

TEST_F(ItemTreeTest, PruneUndefinedNonLeaf)
{
	t5->pruneUndefined();
	ASSERT_EQ(1, t5->getChildren().size());
	EXPECT_EQ(ItemTreeNode::Type::REJECT, (*t5->getChildren().begin())->getNode()->getType());
}

TEST_F(ItemTreeTest, PruneUndefinedThrows)
{
#ifdef DISABLE_CHECKS
#pragma message "Omitting test because DISABLE_CHECKS was defined."
#else
	EXPECT_THROW(t6->pruneUndefined(), std::runtime_error);
#endif
}

TEST_F(ItemTreeTest, EvaluateDoesNotPruneUndefined)
{
	t6->evaluate(true);
	ASSERT_EQ(1, t6->getChildren().size());
}

TEST_F(ItemTreeTest, EvaluatePrunesRejecting)
{
	t7->evaluate(true);
	ASSERT_EQ(1, t7->getChildren().size());
	EXPECT_EQ(ItemTreeNode::Type::UNDEFINED, (*t7->getChildren().begin())->getNode()->getType());
}

TEST_F(ItemTreeTest, EvaluateDoesNotPruneRejecting)
{
	t7->evaluate(false);
	ASSERT_EQ(2, t7->getChildren().size());
}

TEST_F(ItemTreeTest, EvaluateDoesNotPruneRejectingLeaf)
{
	t4->evaluate(false);
	ASSERT_EQ(2, t4->getChildren().size());
}

TEST_F(ItemTreeTest, EvaluateDoesNotPruneDescendantsOfUndefined)
{
	t8->evaluate(true);
	ASSERT_EQ(1, t8->getChildren().size());
	EXPECT_EQ(2, (*t8->getChildren().begin())->getChildren().size());
}

TEST_F(ItemTreeTest, EvaluateSetsHasAcceptingChild)
{
	EXPECT_FALSE(t9->getNode()->getHasAcceptingChild());
	t9->evaluate(true);
	EXPECT_TRUE(t9->getNode()->getHasAcceptingChild());
}

TEST_F(ItemTreeTest, EvaluateSetsHasRejectingChild)
{
	EXPECT_FALSE(t10->getNode()->getHasRejectingChild());
	t10->evaluate(true);
	EXPECT_TRUE(t10->getNode()->getHasRejectingChild());
}

TEST_F(ItemTreeTest, EvaluatePropagatesRejectForAnd)
{
	EXPECT_EQ(ItemTreeNode::Type::REJECT, t9->evaluate(true));
}

TEST_F(ItemTreeTest, EvaluatePropagatesAcceptForOr)
{
	EXPECT_EQ(ItemTreeNode::Type::ACCEPT, t10->evaluate(true));
}

TEST_F(ItemTreeTest, EvaluatePropagatesUndefinedForAnd)
{
	EXPECT_EQ(ItemTreeNode::Type::UNDEFINED, t11->evaluate(true));
}

TEST_F(ItemTreeTest, EvaluatePropagatesUndefinedForOr)
{
	EXPECT_EQ(ItemTreeNode::Type::UNDEFINED, t12->evaluate(true));
}

TEST_F(ItemTreeTest, FinalizeReturnsFalseForNodeWithUndefinedTypeWhenPruningUndefined)
{
	EXPECT_FALSE(undefNode->finalize(app, true, false));
}

TEST_F(ItemTreeTest, FinalizeReturnsTrueForNodeWithDefinedTypeWhenPruningUndefined)
{
	EXPECT_FALSE(rejectNode->finalize(app, true, false));
}

TEST_F(ItemTreeTest, ChildWithEqualCostsIsMerged)
{
	orNode->addChildAndMerge(std::move(t13));
	orNode->addChildAndMerge(std::move(t14));
	EXPECT_EQ(1, orNode->getChildren().size());
}

TEST_F(ItemTreeTest, ChildWithSmallerCostsIsMerged)
{
	andNode->addChildAndMerge(std::move(t13));
	andNode->addChildAndMerge(std::move(t15));
	EXPECT_EQ(1, andNode->getChildren().size());
}

TEST_F(ItemTreeTest, ChildWithSmallerAndEqualCostsIsNotMerged)
{
	orNode->addChildAndMerge(std::move(t13));
	orNode->addChildAndMerge(std::move(t16));
	EXPECT_EQ(2, orNode->getChildren().size());
}

TEST_F(ItemTreeTest, ChildWithGreaterAndSmallerCostsIsNotMerged)
{
	andNode->addChildAndMerge(std::move(t13));
	andNode->addChildAndMerge(std::move(t17));
	EXPECT_EQ(2, andNode->getChildren().size());
}

TEST_F(ItemTreeTest, ChildWithBetterCostButDifferentStructureIsNotMerged)
{
	andNode->addChildAndMerge(std::move(t17));
	andNode->addChildAndMerge(std::move(t18));
	EXPECT_EQ(2, andNode->getChildren().size());
}

// TODO Test clearUnneededExtensionPointers(), prepareChildrenRandomAccess()
