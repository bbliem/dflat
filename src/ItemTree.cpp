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

#include <ostream>

#include "ItemTree.h"

bool ItemTree::TreePtrComparator::operator()(const TreePtr& lhs, const TreePtr& rhs)
{
	return lhs->items < rhs->items || (lhs->items == rhs->items &&
			std::lexicographical_compare(lhs->children.begin(), lhs->children.end(), rhs->children.begin(), rhs->children.end(), *this)
			);
}

ItemTree::ItemTree(Items&& items)
	: items(std::move(items))
{
}

ItemTree::ItemTree(Items&& items, Children&& children)
	: items(std::move(items))
	, children(std::move(children))
{
}

void ItemTree::printNode(std::ostream& os, bool last, std::string indent) const
{
	os << indent;

	if(last) {
		os << "\\-";
		indent += "  ";
	}
	else {
		os << "|-";
		indent += "| ";
	}

	ItemTree::Items::const_iterator it = items.begin();
	if(it != items.end()) {
		os << *it;
		while(++it != items.end())
			os << ',' << *it;
	}
	os << std::endl;

	size_t i = 0;
	for(const auto& child : children)
		child->printNode(os, ++i == children.size(), indent);
}

std::ostream& operator<<(std::ostream& os, const ItemTree& tree)
{
	ItemTree::Items::const_iterator it = tree.items.begin();
	if(it != tree.items.end()) {
			os << *it;
		while(++it != tree.items.end())
			os << ',' << *it;
	}
	os << std::endl;
	size_t i = 0;
	for(const auto& child : tree.children)
		child->printNode(os, ++i == tree.children.size());
	return os;
}
