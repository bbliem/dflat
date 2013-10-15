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

// TODO: Maybe make it a "proper" iterator?
class ExtensionIterator
{
public:
	explicit ExtensionIterator(const ItemTreeNode&);

	//! @return true iff this iterator can be dereferenced
	bool isValid() const { return valid; }

	ItemTreeNode::Items& operator*(); // dereference
	ExtensionIterator& operator++();

private:
	// Reset to initial state after construction
	void reset();
	void resetExtensionPointers();

	// Materialize what will be returned upon dereferencing
	void materializeItems();

	void incrementExtensionIterator(unsigned int i);

	ItemTreeNode::Items items;
	const ItemTreeNode& itemTreeNode;
	bool valid;
	ItemTreeNode::ExtensionPointers::const_iterator curTuple; // current ExtensionPointerTuple in ItemTreeNode
	std::vector<std::unique_ptr<ExtensionIterator>> extensionIts; // one iterator for each node in the extension pointer tuple *curTuple
};
