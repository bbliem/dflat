/*{{{
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
//}}}
#include <memory>
#include <limits>
#include <unordered_map>

#include "DirectedAcyclicGraph.h"
#include "ItemTreeNode.h"

class ItemTree;
typedef std::unique_ptr<ItemTree> ItemTreePtr;
class ExtensionIterator;
class Application;

struct ItemTreePtrComparator { bool operator()(const ItemTreePtr& lhs, const ItemTreePtr& rhs); };

class ItemTree : public DirectedAcyclicGraph<std::shared_ptr<ItemTreeNode>, std::set<ItemTreePtr, ItemTreePtrComparator>>
{
public:
	using DirectedAcyclicGraph::DirectedAcyclicGraph;

	// If there is a subtree rooted at a child of this node that has equal item sets as the given one, the existing subtree is unified with the given one
	// If no merging has occurred (i.e., "subtree" was added as a new child), returns an iterator to this new child; otherwise returns an iterator to this->children.end().
	Children::const_iterator addChildAndMerge(ChildPtr&& subtree);

	// Enables random access to this node's children via getChild(), and also random access for all children of descendants of this node.
	void finalize();

	// Use this after calling finalize() to get the i'th child of this node
	const ItemTree& getChild(size_t i) const;

	// If counting is not required, removes extension pointers of all nodes below (but not including) the materialization depth in order to allow non-extended nodes to be deleted. (They are not going to be needed anymore.)
	// Otherwise, removes extension pointers of all nodes below (but not including) the materialization depth plus one.
	void clearUnneededExtensionPointers(const Application& app, unsigned int currentDepth = 0);

	// Print the tree that would result from recursively extending all nodes
	void printExtensions(std::ostream& os, unsigned int maxDepth = std::numeric_limits<unsigned int>::max(), bool printCount = true, bool root = true, bool lastChild = false, const std::string& indent = "", const ExtensionIterator* parent = nullptr) const;

	// The children of each item tree node are considered ordered.
	// Let A and B be item trees having the same item sets.
	// In ItemTreePtrComparator, A < B holds if there are pairs (a,b) and
	// (a',b') such that:
	//   1. a and a' are nodes of A, while b and b' are nodes of B.
	//   2. a' is the next sibling of a, while b' is the next sibling of b.
	//   3. a and b (and thus also a' and b') correspond to each other, i.e.,
	//      they are at the same depth and there is an integer such that both are
	//      the i'th child of their parent.
	//   4. cost(a) < cost(b) but cost(a') >= cost(b').
	// This method returns true for *this and other having equal item sets if
	// *this < other.
	bool costDifferenceSignIncrease(const ItemTreePtr& other) const;

private:
	// Recursively unify extension pointers of this itree with the other one's given that the item sets are all equal
	void merge(ItemTree&& other);

	std::vector<const ItemTree*> childrenVector; // for random access via getChild()
};

// Key: Global ID of child node; value: the child node's item tree
typedef std::unordered_map<unsigned int, ItemTreePtr> ChildItemTrees;
