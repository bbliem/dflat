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
	int compareSets(const ItemTreeNode::Items& lhs, const ItemTreeNode::Items& rhs)
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
	
        counters["cost"] = type == Type::REJECT ? std::numeric_limits<long>::max() : 0;
	currentCounters["cost"] = 0;
}

void ItemTreeNode::setParent(const ItemTreeNode* parent)
{
	this->parent = parent;
}

//void ItemTreeNode::setCost(long cost)
//{
//#ifndef DISABLE_CHECKS
//	if(type == Type::REJECT)
//		throw std::runtime_error("Tried to set cost of a reject node");
//#endif
//	this->counters["cost"] = cost;
//}

void ItemTreeNode::setCounter(std::string counterName, long counterValue)
{
    if(type != Type::REJECT || counterName.compare("cost") != 0)
        this->counters[counterName] = counterValue;
}

void ItemTreeNode::setCurrentCounter(std::string currentCounterName, long currentCounterValue)
{
	this->currentCounters[currentCounterName] = currentCounterValue;
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
	assert(counters["cost"] == other.counters["cost"]);
	assert(currentCounters["cost"] == other.currentCounters["cost"]);

	// Merge other node's data into this
	extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
	count += other.count;
}

int ItemTreeNode::compareCostInsensitive(const ItemTreeNode& other) const
{
	const int c = compareSets(items, other.items);
	if(c != 0)
		return c;

    for(const auto& counter: counters)
    {
        if(counter.first.compare("cost") != 0)
        {
            if(other.counters.find(counter.first) == other.counters.end())
                return 1;
            else
            {
                if(counter.second < other.counters.at(counter.first))
                    return -1;
                else if(counter.second > other.counters.at(counter.first))
                    return 1;
            }
        }
    }

    for(const auto& counter: other.counters)
    {
        if(counter.first.compare("cost") != 0)
        {
            if(counters.find(counter.first) == counters.end())
                return 1;
            else
            {
                if(counter.second < counters.at(counter.first))
                    return 1;
                else if(counter.second > counters.at(counter.first))
                    return -11;
            }
        }
    }

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
	if(node.getCounter("cost")) {
		os << " (cost: " << node.getCounter("cost");
		if(node.getCurrentCounter("cost")) 
		  os << "; current: " << node.getCurrentCounter("cost");
		os << ')';
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node)
{
	return os << *node;
}
