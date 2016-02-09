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

#include "ItemTreeNode.h"
#include "ExtensionIterator.h"

TEST(ItemTreeNode, ConstructorSetsCount)
{
	// All item tree nodes belong to the following decomposition:
	// n1
	// |- n2
	// |  |- n3
	// \- n4
	//    |- n5

	// Item tree nodes at decomposition node 5: (Count in brackets)
	// n5_a [1]
	// n5_b [1]
	ItemTreeNode::ExtensionPointer n5_a{new ItemTreeNode{{{"n5_a"}}}};
	ItemTreeNode::ExtensionPointer n5_b{new ItemTreeNode{{{"n5_b"}}}};

	// Item tree nodes at decomposition node 4:
	// n4_a [2]
	ItemTreeNode::ExtensionPointer n4_a{new ItemTreeNode{
		{{"n4_a"}},
		{},
		{
			{n5_a},
			{n5_b},
		}
	}};

	// Item tree nodes at decomposition node 3:
	// n3_a [1]
	// n3_b [1]
	// n3_c [1]
	// n3_d [1]
	// n3_e [1]
	ItemTreeNode::ExtensionPointer n3_a{new ItemTreeNode{{{"n3_a"}}}};
	ItemTreeNode::ExtensionPointer n3_b{new ItemTreeNode{{{"n3_b"}}}};
	ItemTreeNode::ExtensionPointer n3_c{new ItemTreeNode{{{"n3_c"}}}};
	ItemTreeNode::ExtensionPointer n3_d{new ItemTreeNode{{{"n3_d"}}}};
	ItemTreeNode::ExtensionPointer n3_e{new ItemTreeNode{{{"n3_e"}}}};

	// Item tree nodes at decomposition node 2:
	// n2_a [3]
	// n2_b [2]
	ItemTreeNode::ExtensionPointer n2_a{new ItemTreeNode{
		{{"n2_a"}},
		{},
		{
			{n3_a},
			{n3_b},
			{n3_c},
		}
	}};
	ItemTreeNode::ExtensionPointer n2_b{new ItemTreeNode{
		{{"n2_b"}},
		{},
		{
			{n3_d},
			{n3_e},
		}
	}};

	// Item tree nodes at decomposition node 1:
	// n1_a [10]
	ItemTreeNode::ExtensionPointer n1_a{new ItemTreeNode{
		{{"n1_a"}},
		{},
		{
			{n2_a, n4_a},
			{n2_b, n4_a},
		}
	}};

	EXPECT_EQ(1, n5_a->getCount());
	EXPECT_EQ(1, n5_b->getCount());

	EXPECT_EQ(2, n4_a->getCount());

	EXPECT_EQ(1, n3_a->getCount());
	EXPECT_EQ(1, n3_b->getCount());
	EXPECT_EQ(1, n3_c->getCount());
	EXPECT_EQ(1, n3_d->getCount());
	EXPECT_EQ(1, n3_e->getCount());

	EXPECT_EQ(3, n2_a->getCount());
	EXPECT_EQ(2, n2_b->getCount());

	EXPECT_EQ(10, n1_a->getCount());
}

TEST(ItemTreeNode, ConstructorRetainsHasAcceptingOrRejectingChild)
{
	ItemTreeNode::ExtensionPointer extended{new ItemTreeNode};
	EXPECT_FALSE(extended->getHasAcceptingChild());
	EXPECT_FALSE(extended->getHasRejectingChild());
	extended->setHasAcceptingChild();
	extended->setHasRejectingChild();
	EXPECT_TRUE(extended->getHasAcceptingChild());
	EXPECT_TRUE(extended->getHasRejectingChild());

	ItemTreeNode extending{{}, {}, {{extended}}};
	EXPECT_TRUE(extending.getHasAcceptingChild());
	EXPECT_TRUE(extending.getHasRejectingChild());
}

TEST(ItemTreeNode, ConstructorThrowsExceptionIfTypeNotRetained)
{
#ifdef DISABLE_CHECKS
#pragma message "Omitting test because DISABLE_CHECKS was defined."
#else
	ItemTreeNode::ExtensionPointer extended{new ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}};
	EXPECT_THROW((ItemTreeNode{{}, {}, {{extended}}}), std::runtime_error);
	EXPECT_THROW((ItemTreeNode{{}, {}, {{extended}}, ItemTreeNode::Type::REJECT}), std::runtime_error);
	EXPECT_NO_THROW((ItemTreeNode{{}, {}, {{extended}}, ItemTreeNode::Type::ACCEPT}));
#endif
}

TEST(ItemTreeNode, ConstructorSetsCost)
{
	EXPECT_EQ(0, (ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::UNDEFINED}.getCost()));
	EXPECT_EQ(0, (ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::OR}.getCost()));
	EXPECT_EQ(0, (ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::AND}.getCost()));
	EXPECT_EQ(0, (ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::ACCEPT}.getCost()));
	EXPECT_EQ(std::numeric_limits<decltype(ItemTreeNode{}.getCost())>::max(), (ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}.getCost()));
}

TEST(ItemTreeNode, MergeUnifiesExtensionPointers)
{
	// c and d extend a and b, respectively
	ItemTreeNode::ExtensionPointer a{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer b{new ItemTreeNode};
	ItemTreeNode c{{{"foo"}}, {}, {{a}}};
	ItemTreeNode d{{{"foo"}}, {}, {{b}}};

	c.merge(std::move(d));
	EXPECT_EQ((ItemTreeNode::ExtensionPointers{{a}, {b}}), c.getExtensionPointers());
}

TEST(ItemTreeNode, MergeAddsCount)
{
	ItemTreeNode a{{{"foo"}}};
	ItemTreeNode b{{{"foo"}}};
	ItemTreeNode c{{{"foo"}}};

	EXPECT_EQ(1, a.getCount());
	a.merge(std::move(b));
	EXPECT_EQ(2, a.getCount());
	a.merge(std::move(c));
	EXPECT_EQ(3, a.getCount());
}

TEST(ItemTreeNode, CompareCostInsensitive)
{
	ItemTreeNode a{{{"a"}}};
	ItemTreeNode b{{{"b"}}};

	EXPECT_EQ(0, a.compareCostInsensitive(a));
	EXPECT_NE(0, a.compareCostInsensitive(b));
	EXPECT_NE(0, b.compareCostInsensitive(a));

	ItemTreeNode aExpensive = a;
	a.setCost(3);
	aExpensive.setCost(5);
	a.setCurrentCost(3);
	aExpensive.setCurrentCost(5);
	EXPECT_EQ(0, a.compareCostInsensitive(aExpensive));
	EXPECT_EQ(0, aExpensive.compareCostInsensitive(a));

	a.setHasAcceptingChild();
	EXPECT_NE(0, aExpensive.compareCostInsensitive(a));
	// Actually we should do similar checks for all other things that change the outcome of the comparison, too...
}

TEST(ItemTreeNode, CountExtensions)
{
	// Decomposition: nodes n1 and n2, where n2 is the child of n1

	// Item tree at n1:
	// n1
	// \- n1_1
	//    \- n1_1_1

	// Item tree at n2:
	// n2
	// |- n2_1
	// |  |- n2_1_1
	// |  \- n2_1_2
	// \- n2_2
	//    \- n2_2_1

	ItemTreeNode::ExtensionPointer n2{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer n2_1{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer n2_1_1{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer n2_1_2{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer n2_2{new ItemTreeNode};
	ItemTreeNode::ExtensionPointer n2_2_1{new ItemTreeNode};
	n2_1  ->setParent(n2  .get());
	n2_1_1->setParent(n2_1.get());
	n2_1_2->setParent(n2_1.get());
	n2_2  ->setParent(n2  .get());
	n2_2_1->setParent(n2_2.get());

	ItemTreeNode::ExtensionPointer n1{new ItemTreeNode{{}, {}, {{n2}}}};
	ItemTreeNode::ExtensionPointer n1_1{new ItemTreeNode{{}, {}, {{n2_1}, {n2_2}}}};
	ItemTreeNode::ExtensionPointer n1_1_1{new ItemTreeNode{{}, {}, {{n2_1_1}, {n2_1_2}, {n2_2_1}}}};
	n1_1  ->setParent(n1  .get());
	n1_1_1->setParent(n1_1.get());

	ExtensionIterator e1{*n1};
	ExtensionIterator e1_1{*n1_1, &e1};

	// e1 points to n2 having two children, so n1_1 has two extensions
	EXPECT_EQ(2, n1_1->countExtensions(e1));

	// e1_1 points to n2_1 having two children, so n1_1_1 has two extensions
	EXPECT_EQ(2, n1_1_1->countExtensions(e1_1));
	EXPECT_EQ(0, n1_1_1->countExtensions(e1)); // does not point to parent of n1_1_1

	++e1_1;
	// e1_1 now points to n2_2 having one child, so n1_1_1 has one extension
	EXPECT_EQ(1, n1_1_1->countExtensions(e1_1));
}

TEST(ItemTreeNode, SetCostThrowsExceptionForRejectNodes)
{
#ifdef DISABLE_CHECKS
#pragma message "Omitting test because DISABLE_CHECKS was defined."
#else
	EXPECT_THROW((ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}.setCost(0)), std::runtime_error);
	EXPECT_THROW((ItemTreeNode{{}, {}, {{}}, ItemTreeNode::Type::REJECT}.setCost(std::numeric_limits<decltype(ItemTreeNode{}.getCost())>::max())), std::runtime_error);
	EXPECT_NO_THROW(ItemTreeNode{}.setCost(0));
#endif
}
