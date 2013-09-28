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

#include "Dummy.h"
#include "../ItemTree.h"
#include "../Decomposition.h"

namespace solver {

ItemTree Dummy::compute()
{
	//ItemTree itree = ItemTreeNode({}); // empty root
	ItemTree itree = ItemTreeNode({std::to_string(decomposition.getRoot().getGlobalId())});

	for(const Decomposition::ChildPtr& child : decomposition.getChildren())
		itree.addChild(ItemTreePtr(new ItemTree(child->getSolver().compute())));

	//itree.addChild(ItemTreePtr(new ItemTree(ItemTreeNode({"foo"}))));
	//itree.addChild(ItemTreePtr(new ItemTree(ItemTreeNode({"bar"}))));

	return itree;
}

} // namespace solver
