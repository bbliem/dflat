/*{{{
Copyright 2012-2014, Bernhard Bliem
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
#include <cassert>
#include <memory>

#include "ItemTreeNode.h"
#include "ExtensionIterator.h"

ItemTreeNode::ItemTreeNode(Items&& items, Items&& auxItems, ExtensionPointers&& extensionPointers, Type type)
	: items(std::move(items))
	, auxItems(std::move(auxItems))
	, extensionPointers(std::move(extensionPointers))
	, type(type)
{
	assert(!this->extensionPointers.empty());
	count = 0;
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		mpz_class product = 1;
		for(const auto& predecessor : tuple)
			product *= predecessor.second->getCount();
		count += product;
	}

#ifndef DISABLE_CHECKS
	for(const ExtensionPointerTuple& tuple : this->extensionPointers)
		for(const auto& predecessor : tuple)
			if(predecessor.second->type != Type::UNDEFINED && type != predecessor.second->type)
				throw std::runtime_error("Type of extended item tree node not retained");
#endif

	// Retain the information about accepting / rejecting children
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		for(const auto& predecessor : tuple) {
			hasAcceptingChild = hasAcceptingChild || predecessor.second->hasAcceptingChild;
			hasRejectingChild = hasRejectingChild || predecessor.second->hasRejectingChild;
		}
	}
}

const ItemTreeNode::Items& ItemTreeNode::getItems() const
{
	return items;
}

const ItemTreeNode::Items& ItemTreeNode::getAuxItems() const
{
	return auxItems;
}

const ItemTreeNode::ExtensionPointers& ItemTreeNode::getExtensionPointers() const
{
	return extensionPointers;
}

void ItemTreeNode::clearExtensionPointers()
{
	extensionPointers.clear();
}

const ItemTreeNode* ItemTreeNode::getParent() const
{
	return parent;
}

void ItemTreeNode::setParent(const ItemTreeNode* parent)
{
	this->parent = parent;
}

const mpz_class& ItemTreeNode::getCount() const
{
	return count;
}

long ItemTreeNode::getCost() const
{
	return cost;
}

void ItemTreeNode::setCost(long cost)
{
	this->cost = cost;
}

long ItemTreeNode::getCurrentCost() const
{
	return currentCost;
}

void ItemTreeNode::setCurrentCost(long currentCost)
{
	this->currentCost = currentCost;
}

ItemTreeNode::Type ItemTreeNode::getType() const
{
	return type;
}

bool ItemTreeNode::getHasAcceptingChild() const
{
	return hasAcceptingChild;
}

void ItemTreeNode::setHasAcceptingChild()
{
	hasAcceptingChild = true;
}

bool ItemTreeNode::getHasRejectingChild() const
{
	return hasRejectingChild;
}

void ItemTreeNode::setHasRejectingChild()
{
	hasRejectingChild = true;
}

mpz_class ItemTreeNode::countExtensions(const ExtensionIterator& parentIterator) const
{
	assert(parentIterator.isValid());
	mpz_class result;
	assert(result == 0);

	if(&parentIterator.getItemTreeNode() == parent) {
		// XXX This is strikingly similar to what's happening in the constructor. What to do?
		if(extensionPointers.empty())
			result = 1;
		else {
			for(const ExtensionPointerTuple& tuple : extensionPointers) {
				mpz_class product = 1;

				assert(parentIterator.getSubIterators().size() == tuple.size());
				ExtensionIterator::SubIterators::const_iterator subIt = parentIterator.getSubIterators().begin();
				for(const auto& predecessor : tuple) {
					product *= predecessor.second->countExtensions(**subIt);
					++subIt;
				}
				result += product;
			}
		}
	}

	return result;
}

void ItemTreeNode::merge(ItemTreeNode&& other)
{
	assert(items == other.items);
	assert(auxItems == other.auxItems);
	assert(cost == other.cost);
	assert(currentCost == other.currentCost);

	// Merge other node's data into this
	extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
	count += other.count;
}

bool ItemTreeNode::compareCostInsensitive(const ItemTreeNode& other) const
{
	// XXX Check that this is not less efficient than a long boolean expression
	return std::tie(items, type, hasAcceptingChild, hasRejectingChild, auxItems) < std::tie(other.items, other.type, other.hasAcceptingChild, other.hasRejectingChild, other.auxItems);
}

std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node)
{
	// Print count
//	os << '[' << node.count << "] ";

	// Print type
	switch(node.type) {
		case ItemTreeNode::Type::UNDEFINED:
			break;
		case ItemTreeNode::Type::OR:
			os << "<OR> ";
			break;
		case ItemTreeNode::Type::AND:
			os << "<AND> ";
			break;
		case ItemTreeNode::Type::ACCEPT:
			os << "<ACCEPT> ";
			break;
		case ItemTreeNode::Type::REJECT:
			os << "<REJECT> ";
			break;
	}

	if(node.hasAcceptingChild)
		os << "(a) ";
	if(node.hasRejectingChild)
		os << "(r) ";

	// Print items
	for(const auto& item : node.items)
		os << item << ' ';
	for(const auto& item : node.auxItems)
		os << item << ' ';

//	os << "; extend: {";
//	std::string tupleSep;
//	for(const auto& tuple : node.extensionPointers) {
//		os << tupleSep << '(';
//		std::string ptrSep;
//		for(const auto& extended : tuple) {
//			os << ptrSep << extended.first << ':' << extended.second.get();
//			ptrSep = ", ";
//		}
//		os << ')';
//		tupleSep = ", ";
//	}
//	os << "}, this: " << &node << ", parent: " << node.parent;

	// Print cost
	if(node.cost != 0)
		os << " (cost: " << node.cost << "; current: " << node.getCurrentCost() << ')';

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node)
{
	return os << *node;
}
