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

#include "ItemTree.h"
#include "ExtensionIterator.h"

bool ItemTreePtrComparator::operator()(const ItemTreePtr& lhs, const ItemTreePtr& rhs)
{
	return lhs->getRoot().getItems() < rhs->getRoot().getItems() || (lhs->getRoot().getItems() == rhs->getRoot().getItems() &&
			std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this)
			);
}

void ItemTree::printExtensions(std::ostream& os, unsigned int maxDepth, bool root, bool lastChild, const std::string& indent) const
{
	ExtensionIterator it(node);

	while(it.isValid()) {
		std::string childIndent = indent;
		os << indent;
		ItemTreeNode::Items items = std::move(*it);
		++it;

		if(!root) {
			if(lastChild && !it.isValid()) {
#ifndef NO_UNICODE
				os << "┗━ ";
				childIndent += "   ";
#else
				os << "\\-";
				childIndent += "  ";
#endif
			}
			else {
#ifndef NO_UNICODE
				os << "┣━ ";
				childIndent += "┃  ";
#else
				os << "|-";
				childIndent += "| ";
#endif
			}
		}

		for(const auto& item : items)
			os << item << ' ';
		os << std::endl;

		if(maxDepth > 0) {
			size_t i = 0;
			for(const auto& child : children)
				child->printExtensions(os, maxDepth - 1, false, ++i == children.size(), childIndent);
		}
	}
}

