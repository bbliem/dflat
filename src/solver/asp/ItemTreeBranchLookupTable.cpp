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

#include "ItemTreeBranchLookupTable.h"

namespace solver { namespace asp {

ItemTreeBranchLookupTable::ItemTreeBranchLookupTable(ItemTree&& itemTree)
	: itemTree(std::move(itemTree))
{
	init(this->itemTree);
}

const ItemTree& ItemTreeBranchLookupTable::getItemTree() const
{
	return itemTree;
}

const ItemTreeBranchLookupTable::Branches& ItemTreeBranchLookupTable::getBranches() const
{
	return branches;
}

const ItemTree& ItemTreeBranchLookupTable::operator[](unsigned int i) const
{
	return *branches.at(i);
}

void ItemTreeBranchLookupTable::init(const ItemTree& node)
{
	if(node.getChildren().empty())
		branches.push_back(&node);
	else {
		for(const auto& child : node.getChildren())
			init(*child);
	}
}

}} // namespace solver::asp
