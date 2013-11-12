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

#include "UncompressedItemTree.h"

namespace solver { namespace asp { namespace trees {

bool UncompressedItemTreePtrComparator::operator()(const UncompressedItemTreePtr& lhs, const UncompressedItemTreePtr& rhs)
{
	return lhs->getRoot()->getItems() < rhs->getRoot()->getItems() ||
		(lhs->getRoot()->getItems() == rhs->getRoot()->getItems() &&
		 (lhs->getRoot()->getType() < rhs->getRoot()->getType() ||
		  (lhs->getRoot()->getType() == rhs->getRoot()->getType() &&
		   (lhs->getRoot()->getHasAcceptingChild() < rhs->getRoot()->getHasAcceptingChild() ||
		    (lhs->getRoot()->getHasAcceptingChild() == rhs->getRoot()->getHasAcceptingChild() &&
		     (lhs->getRoot()->getHasRejectingChild() < rhs->getRoot()->getHasRejectingChild() ||
		      (lhs->getRoot()->getHasRejectingChild() == rhs->getRoot()->getHasRejectingChild() &&
		       (lhs->getRoot()->getAuxItems() < rhs->getRoot()->getAuxItems() ||
		        (lhs->getRoot()->getAuxItems() == rhs->getRoot()->getAuxItems() &&
		         lhs->getRoot()->getExtensionPointers() < rhs->getRoot()->getExtensionPointers())))))))));
}

void UncompressedItemTree::addBranch(Branch::iterator begin, Branch::iterator end)
{
	if(begin != end) {
		std::pair<Children::iterator, bool> result = children.insert(UncompressedItemTreePtr(new UncompressedItemTree(std::move(*begin))));
		(*result.first)->addBranch(++begin, end);
	}
}

ItemTreePtr UncompressedItemTree::compress()
{
	ItemTreePtr result(new ItemTree(std::move(node)));
	for(const auto& child : children)
		result->addChildAndMerge(child->compress());
	return result;
}

}}} // namespace solver::asp::trees
