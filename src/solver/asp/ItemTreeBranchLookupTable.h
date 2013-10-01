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

#pragma once

#include <vector>

#include "../../ItemTree.h"

namespace solver { namespace asp {

// Associates an integer with each branch of an item tree, which allows for random access.
class ItemTreeBranchLookupTable
{
public:
	typedef std::vector<const ItemTree*> Branches;

	ItemTreeBranchLookupTable(ItemTreePtr itemTree);

	ItemTreePtr&& getItemTree();
	const Branches& getBranches() const;

	// Returns the leaf of the i'th branch of the managed item tree
	const ItemTree& operator[](unsigned int i) const;

private:
	void init(const ItemTree& node);

	ItemTreePtr itemTree;
	Branches branches;
};

}} // namespace solver::asp
