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
	return lhs->getRoot()->getItems() < rhs->getRoot()->getItems() ||
		(lhs->getRoot()->getItems() == rhs->getRoot()->getItems() &&
		 (lhs->getRoot()->getConsequentItems() < rhs->getRoot()->getConsequentItems() ||
		  (lhs->getRoot()->getConsequentItems() == rhs->getRoot()->getConsequentItems() &&
		   std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this))));
}

void ItemTree::addChildAndMerge(ChildPtr&& child)
{
	child->parents.push_back(this);
	child->getRoot()->setParent(node.get());
	std::pair<Children::iterator, bool> result = children.insert(std::move(child));
	// XXX If an equivalent element already exists in "children", it is unclear to me whether "child" is actually moved or not. (Maybe it depends on the implementation?)
	// For the time being, pray that it isn't moved in such a case.
	// http://stackoverflow.com/questions/10043716/stdunordered-settinsertt-is-argument-moved-if-it-exists

	if(!result.second) {
		// A subtree rooted at a child with all equal item sets already exists
		assert(child); // XXX See remark above -- child was set to null if it was indeed moved...
		const ItemTreePtr& origChild = *result.first;

		// Unify child with origChild
		child->merge(std::move(*origChild));
		// TODO optimization values as in the old D-FLAT's Row class. (Here or in ItemTreeNode?)
		Children::const_iterator hint = result.first;
		++hint;
		children.erase(result.first);
		children.insert(hint, std::move(child));
		// TODO check that origChild is deleted properly
	}
}

ItemTreeNode::Type ItemTree::prune()
{
	// Can we determine acceptance status already without propagating?
	switch(node->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
		case ItemTreeNode::Type::OR:
		case ItemTreeNode::Type::AND:
			break;

		case ItemTreeNode::Type::ACCEPT:
			assert(children.empty()); // Must only be in leaves
			return ItemTreeNode::Type::ACCEPT;

		case ItemTreeNode::Type::REJECT:
			assert(children.empty()); // Must only be in leaves
			return ItemTreeNode::Type::REJECT;
	}

	// Prune children recursively
	bool allAccepting = true;
	bool allRejecting = true;
	bool someAccepting = false;
	bool someRejecting = false;

	Children::const_iterator it = children.begin();
	while(it != children.end()) {
		ItemTreeNode::Type childResult = (*it)->prune();
		switch(childResult) {
			case ItemTreeNode::Type::OR:
			case ItemTreeNode::Type::AND:
				assert(false); // Only UNDEFINED, ACCEPT or REJECT are allowed
				break;

			case ItemTreeNode::Type::UNDEFINED:
				allRejecting = false;
				allAccepting = false;
				++it;
				break;

			case ItemTreeNode::Type::ACCEPT:
				someAccepting = true;
				allRejecting = false;
				++it;
				break;

			case ItemTreeNode::Type::REJECT:
				someRejecting = true;
				allAccepting = false;
				children.erase(it++); // Remove that child
				break;
		}
	}

	// Determine acceptance status of this configuration, if possible
	switch(node->getType()) {
		case ItemTreeNode::Type::OR:
			if(someAccepting)
				return ItemTreeNode::Type::ACCEPT;
			if(allRejecting)
				return ItemTreeNode::Type::REJECT;
			break;

		case ItemTreeNode::Type::AND:
			if(allAccepting)
				return ItemTreeNode::Type::ACCEPT;
			if(someRejecting)
				return ItemTreeNode::Type::REJECT;
			break;

		default:
			break;
	}

	return ItemTreeNode::Type::UNDEFINED;
}

void ItemTree::finalize()
{
	assert(childrenVector.empty());
	childrenVector.reserve(children.size());
	for(const auto& child : children) {
		childrenVector.push_back(child.get());
		child->finalize();
	}
}

const ItemTree& ItemTree::getChild(size_t i) const
{
	assert(childrenVector.size() == children.size());
	assert(i < childrenVector.size());
	return *childrenVector[i];
}

void ItemTree::printExtensions(std::ostream& os, unsigned int maxDepth, bool root, bool lastChild, const std::string& indent, const ExtensionIterator* parent) const
{
	std::unique_ptr<ExtensionIterator> it(new ExtensionIterator(*node, parent));

	while(it->isValid()) {
		std::string childIndent = indent;
		os << indent;
		ItemTreeNode::Items items = std::move(**it);

		// XXX All this ugly stuff just for pretty printing... (Otherwise ExtensionIterator would not need to be copyable and it might use unique_ptr instead of shared_ptr internally.)
		std::unique_ptr<ExtensionIterator> currentIt(new ExtensionIterator(*it));
		++*it;

		if(!root) {
			if(lastChild && !it->isValid()) {
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

		// When limiting the depth causes children not to be extended, print the number of accepting children
		if(maxDepth == 0 && children.empty() == false) {
			os << '[';
			mpz_class count;
			assert(count == 0);
			// On the first level we can use the counts inside the nodes
			// XXX This redundancy is a bit ugly but maybe offers better performance than recalculating the number of accepting children (like below in the "else" branch).
			if(!parent) {
				for(const auto& child : children)
					count += child->node->getCount();
			}
			else {
				for(const auto& child : children)
					count += child->node->countExtensions(*currentIt);
			}
			os << count << "] ";
		}

		for(const auto& item : items)
			os << item << ' ';

		// Print cost
		if(node->getCost() != 0)
			os << "(cost: " << node->getCost() << ')';

		os << std::endl;

		if(maxDepth > 0) {
			size_t i = 0;
			for(const auto& child : children)
				child->printExtensions(os, maxDepth - 1, false, ++i == children.size(), childIndent, currentIt.get());
		}
	}
}

void ItemTree::merge(const ItemTree& other)
{
	assert(node->getItems() == other.node->getItems());
	assert(node->getConsequentItems() == other.node->getConsequentItems());
	node->merge(std::move(*other.node));
	assert(children.size() == other.children.size());
	Children::const_iterator it = other.children.begin();
	for(const ItemTreePtr& child : children) {
		assert(it != other.children.end());
		child->merge(**it);
		++it;
	}
}
