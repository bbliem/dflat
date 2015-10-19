/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <gmpxx.h>

#include "String.h"

class ExtensionIterator;

// IMPORTANT NOTE: Remember that when you change something here it might
// require changing other classes like ItemTreePtrComparator (and
// solver::clasp::trees::UncompressedItemTreePtrComparator).

class ItemTreeNode
{
public:
	typedef std::set<String> Items; // We need the sortedness for, e.g., the default join.
	typedef std::shared_ptr<ItemTreeNode> ExtensionPointer;
	typedef std::vector<ExtensionPointer> ExtensionPointerTuple;
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;

	enum class Type {
		UNDEFINED,
		OR,
		AND,
		ACCEPT,
		REJECT
	};

	// extensionPointers may not be empty; if there are no decomposition
	// children, it should contain a 0-tuple.
	// Sets the count to the sum of the products of the counts for each
	// extension pointer tuple.
	// Sets hasAcceptingChild and hasRejectingChild to true if any extended
	// node has set the respective flag to true.
	// Unless checks are disabled, throws an exception if any extended node has
	// a defined type and the given node type is different from this.
	// If the type is REJECT, sets the cost to the highest possible value,
	// otherwise to 0.
	ItemTreeNode(Items&& items = {}, Items&& auxItems = {}, ExtensionPointers&& extensionPointers = {{}}, Type type = Type::UNDEFINED);

	// Returns the items of this node (but not the auxiliary items, see below).
	const Items& getItems() const { return items; }

	// Returns the items that have been declared as auxiliary.
	// These items are disregarded in the default join.
	const Items& getAuxItems() const { return auxItems; }

	const ExtensionPointers& getExtensionPointers() const { return extensionPointers; }
	void clearExtensionPointers() { extensionPointers.clear(); }

	const ItemTreeNode* getParent() const { return parent; }
	void setParent(const ItemTreeNode*);

	const mpz_class& getCount() const { return count; }

	long getCost() const { return cost; }
	// Throws a runtime_error if this is a REJECT node.
	void setCost(long cost);

	long getCurrentCost() const { return currentCost; }
	void setCurrentCost(long currentCost);

	Type getType() const { return type; }

	bool getHasAcceptingChild() const { return hasAcceptingChild; }
	void setHasAcceptingChild();
	bool getHasRejectingChild() const { return hasRejectingChild; }
	void setHasRejectingChild();

	// Calculate the number of extensions of this node given an iterator pointing to an extension of this node's parent.
	// This is different from getCount() since getCount() returns the number of extensions for *any* possible extension of the parent.
	// This method traverses the entire decomposition.
	// If parentIterator does not point to this node's parent, returns 0.
	// parentIterator must be valid.
	mpz_class countExtensions(const ExtensionIterator& parentIterator) const;

	// Unify extension pointers of this node with the other one's given that the item sets are equal.
	// "other" will subsequently be thrown away and only "this" will be retained.
	void merge(ItemTreeNode&& other);

	// Returns a negative/positive integer if this is "less"/"greater" than other, without considering costs.
	// Returns zero if this is equal to other, without considering costs.
	// Only considers items, type, hasAcceptingChild, hasRejectingChild and auxItems.
	int compareCostInsensitive(const ItemTreeNode& other) const;

	// Estimates the size of this item tree node
	// XXX If size_t has, e.g., 32 bits, we can only measure up to 4 GiB
	std::size_t estimateSize() const;

	// Print this node (no newlines)
	friend std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node);

private:
	Items items;
	Items auxItems;
	ExtensionPointers extensionPointers;
	const ItemTreeNode* parent = nullptr;
	mpz_class count; // number of possible extensions of this node
	long cost = 0;
	long currentCost = 0;
	Type type;
	// Whether this node has a child whose acceptance status (either due to ACCEPT/REJECT in leaves or propagation in AND/OR nodes) is ACCEPT or REJECT, respectively.
	// We must keep track of this explicitly since accepting/rejecting children might have been pruned.
	bool hasAcceptingChild = false;
	bool hasRejectingChild = false;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node);
