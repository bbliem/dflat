/*
Copyright 2012-2013, Bernhard Bliem
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

#include <memory>
#include <limits>

#include "DirectedAcyclicGraph.h"
#include "ItemTreeNode.h"

class ItemTree;
typedef std::unique_ptr<ItemTree> ItemTreePtr;
class ExtensionIterator;

// The set of children is sorted ascendingly according to the following criterion:
// An ItemTreePtr is smaller than another if
// (a) its item set is (lexicographically) smaller, or
// (b) its item set is equal to the other's and its set of children is (lexicographically) smaller.
struct ItemTreePtrComparator { bool operator()(const ItemTreePtr& lhs, const ItemTreePtr& rhs); };

class ItemTree : public DirectedAcyclicGraph<std::shared_ptr<ItemTreeNode>, std::set<ItemTreePtr, ItemTreePtrComparator>>
{
public:
	using DirectedAcyclicGraph::DirectedAcyclicGraph;

	// If there is a subtree rooted at a child of this node that has equal item sets as the given one, the existing subtree is unified with the given one
	void addChildAndMerge(ChildPtr&& child);

	// This propagates acceptance statuses from the leaves toward this node and prunes in the course of this if children are found to be rejecting. If this configuration can be determined to be accepting or rejecting, returns ACCEPT or REJECT, respectively; otherwise returns UNDEFINED.
	// Do not call this after finalize().
	ItemTreeNode::Type prune();

	// Enables random access to this node's children via getChild(), and also random access for all children of descendants of this node.
	// Propagates costs from the leaves toward this node.
	void finalize();

	// Use this after calling finalize() to get the i'th child of this node
	const ItemTree& getChild(size_t i) const;

	// Print the tree that would result from recursively extending all nodes
	void printExtensions(std::ostream& os, unsigned int maxDepth = std::numeric_limits<unsigned int>::max(), bool root = true, bool lastChild = false, const std::string& indent = "", const ExtensionIterator* parent = nullptr) const;

private:
	// Recursively unify extension pointers of this itree with the other one's given that the item sets are all equal
	void merge(const ItemTree& other);

	std::vector<const ItemTree*> childrenVector; // for random access via getChild()
};
