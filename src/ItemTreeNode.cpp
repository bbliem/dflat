/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
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
#include <limits>

#include "ItemTreeNode.h"
#include "ExtensionIterator.h"

namespace {
	// Returns a negative integer if lhs < rhs, a positive integer if rhs > lhs, 0 if lhs == rhs
	template<typename T>
	int compareSets(const T& lhs, const T& rhs)
	{
		const size_t smallestSize = std::min(lhs.size(), rhs.size());
		size_t i = 0;
		for(auto it1 = lhs.begin(), it2 = rhs.begin(); i < smallestSize; ++it1, ++it2, ++i) {
			if(*it1 < *it2)
				return -1;
			else if(*it1 > *it2)
				return 1;
		}
		return lhs.size() - rhs.size();
	}
}

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
			product *= predecessor->getCount();
		count += product;
	}

#ifndef DISABLE_CHECKS
	for(const ExtensionPointerTuple& tuple : this->extensionPointers)
		for(const auto& predecessor : tuple)
			if(predecessor->type != Type::UNDEFINED && type != predecessor->type)
				throw std::runtime_error("Type of extended item tree node not retained");
#endif

	// Retain the information about accepting / rejecting children
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		for(const auto& predecessor : tuple) {
			hasAcceptingChild = hasAcceptingChild || predecessor->hasAcceptingChild;
			hasRejectingChild = hasRejectingChild || predecessor->hasRejectingChild;
		}
	}

	cost = type == Type::REJECT ? std::numeric_limits<decltype(cost)>::max() : 0;
	currentCost = 0;
}

void ItemTreeNode::setParent(const ItemTreeNode* parent)
{
	this->parent = parent;
}

void ItemTreeNode::setCost(long cost)
{
#ifndef DISABLE_CHECKS
	if(type == Type::REJECT)
		throw std::runtime_error("Tried to set cost of a reject node");
#endif
	this->cost = cost;
}

void ItemTreeNode::setCurrentCost(long currentCost)
{
	this->currentCost = currentCost;
}

long ItemTreeNode::getCounter(const std::string& counterName) const
{
	Counters::const_iterator it = counters.find(counterName);
	return it == counters.end() ? 0 : it->second;
}

void ItemTreeNode::setCounter(const std::string& counterName, long counterValue)
{
	assert(counterName != "cost");
	counters[counterName] = counterValue;
}

long ItemTreeNode::getCurrentCounter(const std::string& currentCounterName) const
{
	CurrentCounters::const_iterator it = currentCounters.find(currentCounterName);
	return it == currentCounters.end() ? 0 : it->second;
}

void ItemTreeNode::setCurrentCounter(const std::string& currentCounterName, long currentCounterValue)
{
	assert(currentCounterName != "cost");
	currentCounters[currentCounterName] = currentCounterValue;
}

void ItemTreeNode::setHasAcceptingChild()
{
	hasAcceptingChild = true;
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
					product *= predecessor->countExtensions(**subIt);
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

int ItemTreeNode::compareCostInsensitive(const ItemTreeNode& other) const
{
	int c = compareSets(items, other.items);
	if(c != 0)
		return c;

	if(type < other.type)
		return -1;
	else if(type > other.type)
		return 1;

	if(hasAcceptingChild < other.hasAcceptingChild)
		return -1;
	else if(hasAcceptingChild > other.hasAcceptingChild)
		return 1;

	if(hasRejectingChild < other.hasRejectingChild)
		return -1;
	else if(hasRejectingChild > other.hasRejectingChild)
		return 1;

	c = compareSets(counters, other.counters);
	if(c != 0)
		return c;

	return compareSets(auxItems, other.auxItems);
}

ItemTreeNode::Items ItemTreeNode::firstExtension() const
{
	Items result = items;
	assert(extensionPointers.size() > 0);
	ExtensionPointerTuple ept = extensionPointers.front();
	for(const ExtensionPointer& ep : ept) {
		const Items childResult = ep->firstExtension();
		result.insert(childResult.begin(), childResult.end());
	}
	return result;
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
	if(node.cost != 0) {
		os << " (cost: " << node.cost;
		if(node.getCurrentCost() != 0)
			os << "; current: " << node.getCurrentCost();
		os << ')';
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node)
{
	return os << *node;
}
