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

#include <cassert>
#include <memory>

#include "ItemTreeNode.h"
#include "ExtensionIterator.h"

ItemTreeNode::ItemTreeNode(Items&& items, ExtensionPointers&& extensionPointers)
	: items(std::move(items))
	, extensionPointers(std::move(extensionPointers))
	, parent(nullptr)
{
	if(this->extensionPointers.empty())
		count = 1;
	else {
		count = 0;
		for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
			mpz_class product = 1;
			for(const auto& predecessor : tuple)
				product *= predecessor.second->getCount();
			count += product;
		}
	}

	// TODO for performance, we could recognize these special items during grounding and then set the type from ClaspCallback instead of always going through the items
	assert(std::count_if(this->items.begin(), this->items.end(), [](const std::string& item){
		return item == "_or" || item == "_and" || item == "_accept" || item == "_reject";
	}) <= 1);
	if(this->items.find("_or") != this->items.end())
		type = Type::OR;
	else if(this->items.find("_and") != this->items.end())
		type = Type::AND;
	else if(this->items.find("_accept") != this->items.end())
		type = Type::ACCEPT;
	else if(this->items.find("_reject") != this->items.end())
		type = Type::REJECT;
}

const ItemTreeNode::Items& ItemTreeNode::getItems() const
{
	return items;
}

const ItemTreeNode::ExtensionPointers& ItemTreeNode::getExtensionPointers() const
{
	return extensionPointers;
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

int ItemTreeNode::getCost() const
{
	return cost;
}

void ItemTreeNode::setCost(int cost)
{
	this->cost = cost;
}

ItemTreeNode::Type ItemTreeNode::getType() const
{
	return type;
}

mpz_class ItemTreeNode::countExtensions(const ExtensionIterator& parentIterator) const
{
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

	if(other.cost < cost) {
		// Throw away this node's data and retain the other one's
		extensionPointers.swap(other.extensionPointers);
		count = other.count;
		cost = other.cost;
	}
	else if(other.cost == cost) {
		// Merge other node's data into this
		extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
		count += other.count;
	}
}

std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node)
{
	// Print count
	os << '[' << node.count << "] ";

	// Print items
	ItemTreeNode::Items::const_iterator it = node.items.begin();
	if(it != node.items.end()) {
		os << *it;
		while(++it != node.items.end())
			os << ' ' << *it;
	}

//	for(const auto& tuple : node.extensionPointers) {
//		os << " (";
//		for(const auto& extended : tuple)
//			os << extended.first << ':' << extended.second.get() << ',';
//		os << ") ";
//	}
//	os << "(this: " << &node << ", parent: " << node.parent << ")";

	// Print cost
	if(node.cost != 0)
		os << " (cost: " << node.cost << ')';

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<ItemTreeNode>& node)
{
	return os << *node;
}
