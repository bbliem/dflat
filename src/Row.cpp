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

#include "Row.h"

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

Row::Row(Items&& items, Items&& auxItems, ExtensionPointers&& extensionPointers)
	: items(std::move(items))
	, auxItems(std::move(auxItems))
	, extensionPointers(std::move(extensionPointers))
{
	assert(!this->extensionPointers.empty());
	count = 0;
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		mpz_class product = 1;
		for(const auto& predecessor : tuple)
			product *= predecessor->getCount();
		count += product;
	}

	cost = 0;
	currentCost = 0;
}

void Row::setCost(long cost)
{
	this->cost = cost;
}

void Row::setCurrentCost(long currentCost)
{
	this->currentCost = currentCost;
}

void Row::merge(Row&& other)
{
	assert(items == other.items);
	assert(auxItems == other.auxItems);
	assert(cost == other.cost);
	assert(currentCost == other.currentCost);

	// Merge other row's data into this
	extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
	count += other.count;
}

int Row::compareCostInsensitive(const Row& other) const
{
	int c = compareSets(items, other.items);
	if(c != 0)
		return c;

	return compareSets(auxItems, other.auxItems);
}

Row::Items Row::firstExtension() const
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

std::ostream& operator<<(std::ostream& os, const Row& row)
{
	// Print count
//	os << '[' << row.count << "] ";

	// Print items
	for(const auto& item : row.items)
		os << item << ' ';
	for(const auto& item : row.auxItems)
		os << item << ' ';

//	os << "; extend: {";
//	std::string tupleSep;
//	for(const auto& tuple : row.extensionPointers) {
//		os << tupleSep << '(';
//		std::string ptrSep;
//		for(const auto& extended : tuple) {
//			os << ptrSep << extended.first << ':' << extended.second.get();
//			ptrSep = ", ";
//		}
//		os << ')';
//		tupleSep = ", ";
//	}
//	os << "}, this: " << &row << ", parent: " << row.parent;

	// Print cost
	if(row.cost != 0) {
		os << " (cost: " << row.cost;
		if(row.getCurrentCost() != 0)
			os << "; current: " << row.getCurrentCost();
		os << ')';
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Row>& row)
{
	return os << *row;
}
