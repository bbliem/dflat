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

#include "ItemTree.h"
#include "ExtensionIterator.h"

bool ItemTreePtrComparator::operator()(const ItemTreePtr& lhs, const ItemTreePtr& rhs)
{
	return lhs->getRoot().getItems() < rhs->getRoot().getItems() || (lhs->getRoot().getItems() == rhs->getRoot().getItems() &&
			std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this)
			);
}

void ItemTree::addChildAndMerge(ChildPtr&& child)
{
	child->parents.push_back(this);
	std::pair<Children::iterator, bool> result = children.insert(std::move(child));
	// XXX If an equivalent element already exists in "children", it is unclear to me whether "child" is actually moved or not. (Maybe it depends on the implementation?)
	// For the time being, pray that it isn't moved in such a case.
	// http://stackoverflow.com/questions/10043716/stdunordered-settinsertt-is-argument-moved-if-it-exists
	if(!result.second) {
		// A subtree rooted at a child with all equal item sets already exists
		assert(child); // XXX See remark above -- child was set to null if it was indeed moved...
		const ItemTreePtr& origChild = *result.first;
		// Unify child with origChild
		child->merge(*origChild);
		// TODO optimization values as in the old D-FLAT's Row class. (Here or in ItemTreeNode?)
		Children::const_iterator hint = result.first;
		++hint;
		children.erase(result.first);
		children.insert(hint, std::move(child));
		// TODO check that origChild is deleted properly
	}
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

void ItemTree::merge(const ItemTree& other)
{
	assert(node.getItems() == other.node.getItems());
	node.merge(other.node);
	assert(children.size() == other.children.size());
	Children::const_iterator it = other.children.begin();
	for(const ItemTreePtr& child : children) {
		assert(it != other.children.end());
		child->merge(**it);
		++it;
	}
}
