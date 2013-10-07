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
	if(lhs->getRoot()->getItems() < rhs->getRoot()->getItems())
		return true;
	if(lhs->getRoot()->getItems() == rhs->getRoot()->getItems()) {
		if(lhs->getRoot()->getExtensionPointers() < rhs->getRoot()->getExtensionPointers())
			return true;
		if(lhs->getRoot()->getExtensionPointers() == rhs->getRoot()->getExtensionPointers())
			return std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this);
	}
	return false;
}

void UncompressedItemTree::addBranch(Branch::iterator begin, Branch::iterator end)
{
	if(begin != end) {
#ifndef NDEBUG
		Children::iterator it = children.find(*begin);
		assert(it == children.end() || (!UncompressedItemTreePtrComparator()(*begin, *it) && !UncompressedItemTreePtrComparator()(*it, *begin)));
#endif
		std::pair<Children::iterator, bool> result = children.insert(std::move(*begin));
		(*result.first)->addBranch(++begin, end);
	}
}

}}} // namespace solver::asp::trees
