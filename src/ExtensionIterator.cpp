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

ExtensionIterator::ExtensionIterator(const ItemTreeNode& itemTreeNode, const ExtensionIterator* parent)
	: itemTreeNode(itemTreeNode)
	, parent(parent)
{
	reset();
}

bool ExtensionIterator::hasNext() const
{
	assert(valid);
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	for(const auto& it : subIts)
		if(it->hasNext() == true)
			return true;

	return std::next(curTuple) != itemTreeNode.getExtensionPointers().end();
}

bool ExtensionIterator::curTupleAreChildrenOfParent() const
{
	assert(parent);
	assert(curTuple != itemTreeNode.getExtensionPointers().end());
	assert(parent->curTuple != parent->itemTreeNode.getExtensionPointers().end());
	assert(curTuple->size() == parent->curTuple->size());

	ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
	for(const auto& parentExtension : *parent->curTuple) {
		if(tupleIt->second->getParent() != parentExtension.second.get())
			return false;
		++tupleIt;
	}
	return true;
}

bool ExtensionIterator::forwardCurTuple()
{
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	while(curTupleAreChildrenOfParent() == false) {
		++curTuple;
		if(curTuple == itemTreeNode.getExtensionPointers().end()) {
			// itemTreeNode has no extension pointer tuple such that each of its extension pointers refers to a child of parent
			valid = false;
			return false;
		}
	}
	return true;
}

void ExtensionIterator::reset()
{
	valid = true;
	curTuple = itemTreeNode.getExtensionPointers().begin();
	assert(curTuple != itemTreeNode.getExtensionPointers().end()); // If there are no decomposition children, *curTuple should be a vector of size 0
	assert(curTuple->empty() == false || subIts.empty()); // If the current tuple is empty, there should also be no subIts

	if(parent && forwardCurTuple() == false)
		return;

	subIts.resize(curTuple->size());
	resetExtensionPointers();

	materializeItems();
}

void ExtensionIterator::resetExtensionPointers()
{
	assert(curTuple != itemTreeNode.getExtensionPointers().end());
	assert(curTuple->size() == subIts.size());

	if(parent) {
		assert(curTuple->size() == parent->subIts.size());
		ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
		for(unsigned i = 0; i < curTuple->size(); ++i) {
			subIts[i].reset(new ExtensionIterator(*tupleIt->second, parent->subIts[i].get()));
			++tupleIt;
		}
	}
	else {
		ItemTreeNode::ExtensionPointerTuple::const_iterator tupleIt = curTuple->begin();
		for(unsigned i = 0; i < curTuple->size(); ++i) {
			subIts[i].reset(new ExtensionIterator(*tupleIt->second));
			++tupleIt;
		}
	}
}

void ExtensionIterator::materializeItems()
{
	items.clear();
	items.insert(itemTreeNode.getItems().begin(), itemTreeNode.getItems().end());
	items.insert(itemTreeNode.getAuxItems().begin(), itemTreeNode.getAuxItems().end());
	for(const auto& it : subIts) {
		assert(it);
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
	assert(curTuple != itemTreeNode.getExtensionPointers().end());

	incrementExtensionIterator(0);
	if(valid)
		materializeItems();

	return *this;
}

const ExtensionIterator* ExtensionIterator::getParentIterator() const
{
	return parent;
}

const ItemTreeNode& ExtensionIterator::getItemTreeNode() const
{
	return itemTreeNode;
}

const ExtensionIterator::SubIterators& ExtensionIterator::getSubIterators() const
{
	return subIts;
}

void ExtensionIterator::incrementExtensionIterator(unsigned int i) {
	if(i == subIts.size()) {
		// The last one was the rightmost extension iterator.
		// Use next extension pointer tuple.
		++curTuple;
		if(curTuple == itemTreeNode.getExtensionPointers().end())
			valid = false;
		else {
			if(forwardCurTuple())
				resetExtensionPointers();
		}
	}
	else {
		++(*subIts[i]);
		if(!subIts[i]->valid) {
			// Now we need to advance the next iterator left of it ("carry operation")
			subIts[i]->reset();
			incrementExtensionIterator(i + 1);
		}
	}
}
