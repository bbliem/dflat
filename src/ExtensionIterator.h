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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>

#include "ItemTreeNode.h"

class ExtensionIterator
{
public:
	typedef std::vector<std::shared_ptr<ExtensionIterator>> SubIterators; // XXX unique_ptr would be more efficient, but then we can't copy the iterator -- but this is currently not strictly necessary anyway (except for pretty-printing)

	explicit ExtensionIterator(const ItemTreeNode&, const ExtensionIterator* parent = nullptr);

	//! @return true iff this iterator can be dereferenced
	bool isValid() const { return valid; }
	bool hasNext() const;

	ItemTreeNode::Items& operator*(); // dereference
	ExtensionIterator& operator++(); // Point to next child of what results from dereferencing "parent"

	const ExtensionIterator* getParentIterator() const;
	const ItemTreeNode& getItemTreeNode() const;
	const SubIterators& getSubIterators() const;

private:
	bool curTupleAreChildrenOfParent() const;
	bool forwardCurTuple();

	// Reset to initial state after construction
	void reset();
	void resetExtensionPointers();

	// Materialize what will be returned upon dereferencing
	void materializeItems();

	void incrementExtensionIterator(unsigned int i);

	ItemTreeNode::Items items;
	const ItemTreeNode& itemTreeNode;
	const ExtensionIterator* parent;
	bool valid;
	ItemTreeNode::ExtensionPointers::const_iterator curTuple; // current ExtensionPointerTuple in ItemTreeNode

	SubIterators subIts; // one iterator for each node in the extension pointer tuple *curTuple
};
