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

#pragma once
//}}}
#include <memory>

#include "DirectedAcyclicGraph.h"
#include "ItemTreeNode.h"
#include "ItemTree.h"

class UncompressedItemTree;
typedef std::unique_ptr<UncompressedItemTree> UncompressedItemTreePtr;

// The set of children is sorted ascendingly according to the following criterion:
// An UncompressedItemTreePtr is smaller than another if
// (a) its item set is (lexicographically) smaller, or
// (b) its item set is equal to the other's and its set of extension pointers is (lexicographically) smaller
struct UncompressedItemTreePtrComparator { bool operator()(const UncompressedItemTreePtr& lhs, const UncompressedItemTreePtr& rhs); };

class UncompressedItemTree : public DirectedAcyclicGraph<std::shared_ptr<ItemTreeNode>, std::set<UncompressedItemTreePtr, UncompressedItemTreePtrComparator>>
{
public:
	typedef std::vector<Node> Branch;

	using DirectedAcyclicGraph::DirectedAcyclicGraph;

	// Moves the given branch.
	// *begin will be a child of *this.
	void addBranch(Branch::iterator begin, Branch::iterator end);

	// Merges sibling subtrees with equal item sets. Moves each node in this tree to the resulting item tree.
	// Also propagates costs from leaves to root.
	// To be precise, if ignoreUndefCost is false, an OR (AND) node's cost is set to the minimum (maximum) cost of its children.
	// If ignoreUndefCost is true, that node's cost is instead set to the minimum (maximum) cost among the children whose type is not undefined.
	// Do not call this before prune().
	ItemTreePtr compress(bool ignoreUndefCost);

private:
#ifndef NDEBUG
	void printDebug() const;
#endif
};
