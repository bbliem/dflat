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

#include "ItemTreeNode.h"

ItemTreeNode::ItemTreeNode(Items&& items)
	: items(std::move(items))
{
}

const ItemTreeNode::Items& ItemTreeNode::getItems() const
{
	return items;
}

std::ostream& operator<<(std::ostream& os, const ItemTreeNode& node)
{
	ItemTreeNode::Items::const_iterator it = node.items.begin();
	if(it != node.items.end()) {
		os << *it;
		while(++it != node.items.end())
			os << ',' << *it;
	}
	return os;
}
