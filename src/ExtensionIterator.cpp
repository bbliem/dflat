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

#include <cassert>

#include "ExtensionIterator.h"

ExtensionIterator::ExtensionIterator(const ItemTreeNode& itemTreeNode)
: itemTreeNode(itemTreeNode), valid(true), curTuple(itemTreeNode.getExtensionPointers().begin())
{
	if(curTuple != itemTreeNode.getExtensionPointers().end()) {
		extensionIts.reserve(curTuple->size());
		for(const auto& extendedItemTreeNode : *curTuple)
			extensionIts.emplace_back(new ExtensionIterator(*extendedItemTreeNode));
	}
	materializeItems();
}

void ExtensionIterator::reset()
{
	curTuple = itemTreeNode.getExtensionPointers().begin();
	valid = true;
	resetExtensionPointers();
}

void ExtensionIterator::resetExtensionPointers()
{
	if(curTuple != itemTreeNode.getExtensionPointers().end()) {
		assert(curTuple->size() == extensionIts.size());

		for(unsigned i = 0; i < curTuple->size(); ++i)
			extensionIts[i].reset(new ExtensionIterator(*(*curTuple)[i]));
	}
}

void ExtensionIterator::materializeItems()
{
	items.clear();
	items.insert(itemTreeNode.getItems().begin(), itemTreeNode.getItems().end());
	for(const auto& it : extensionIts) {
		const ItemTreeNode::Items& extension = **it;
		items.insert(extension.begin(), extension.end());
	}
}

ItemTreeNode::Items& ExtensionIterator::operator*()
{
	assert(valid);
	return items;
}

ExtensionIterator& ExtensionIterator::operator++()
{
	assert(valid);

	if(curTuple == itemTreeNode.getExtensionPointers().end())
		valid = false;
	else {
		incrementExtensionIterator(0);
		if(valid)
			materializeItems();
	}

	return *this;
}

void ExtensionIterator::incrementExtensionIterator(unsigned int i) {
	if(i == extensionIts.size()) {
		// The last one was the rightmost extension iterator.
		// Use next extension pointer tuple.
		++curTuple;
		if(curTuple == itemTreeNode.getExtensionPointers().end())
			valid = false;
		else
			resetExtensionPointers();
	}
	else {
		++(*extensionIts[i]);
		if(!extensionIts[i]->valid) {
			// Now we need to advance the next iterator left of it ("carry operation")
			extensionIts[i]->reset();
			extensionIts[i]->materializeItems();
			incrementExtensionIterator(i + 1);
		}
	}
}
