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
		 (lhs->getRoot()->getType() < rhs->getRoot()->getType() ||
		  (lhs->getRoot()->getType() == rhs->getRoot()->getType() &&
		   (lhs->getRoot()->getHasAcceptingChild() < rhs->getRoot()->getHasAcceptingChild() ||
		    (lhs->getRoot()->getHasAcceptingChild() == rhs->getRoot()->getHasAcceptingChild() &&
		     (lhs->getRoot()->getHasRejectingChild() < rhs->getRoot()->getHasRejectingChild() ||
		      (lhs->getRoot()->getHasRejectingChild() == rhs->getRoot()->getHasRejectingChild() &&
		       (lhs->getRoot()->getAuxItems() < rhs->getRoot()->getAuxItems() ||
		        (lhs->getRoot()->getAuxItems() == rhs->getRoot()->getAuxItems() &&
		         std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this))))))))));
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
		Children::const_iterator hint = result.first;
		++hint;
		children.erase(result.first);
		children.insert(hint, std::move(child));
	}
}

ItemTreeNode::Type ItemTree::prune()
{
	// Prune children recursively
	bool allAccepting = true;
	bool allRejecting = true;

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
				node->setHasAcceptingChild();
				allRejecting = false;
				++it;
				break;

			case ItemTreeNode::Type::REJECT:
				node->setHasRejectingChild();
				allAccepting = false;
				// Remove that child
				children.erase(it++);
				break;
		}
	}

	// Determine acceptance status of this configuration, if possible
	switch(node->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
			break;

		case ItemTreeNode::Type::OR:
			if(node->getHasAcceptingChild())
				return ItemTreeNode::Type::ACCEPT;
			if(allRejecting)
				return ItemTreeNode::Type::REJECT;
			break;

		case ItemTreeNode::Type::AND:
			if(allAccepting)
				return ItemTreeNode::Type::ACCEPT;
			if(node->getHasRejectingChild())
				return ItemTreeNode::Type::REJECT;
			break;

		case ItemTreeNode::Type::ACCEPT:
			assert(children.empty()); // Must only be in leaves
			return ItemTreeNode::Type::ACCEPT;

		case ItemTreeNode::Type::REJECT:
			assert(children.empty()); // Must only be in leaves
			return ItemTreeNode::Type::REJECT;
	}

	return ItemTreeNode::Type::UNDEFINED;
}

void ItemTree::finalize()
{
	if(children.empty() == false) {
		// Fill children vector for random access to children
		// Propagate cost of children toward this node
		assert(childrenVector.empty());
		childrenVector.reserve(children.size());
		const bool maximize = node->getType() == ItemTreeNode::Type::AND;
		assert(node->getCost() == 0);
		node->setCost(maximize ? std::numeric_limits<long>::min() : std::numeric_limits<long>::max());
		for(const auto& child : children) {
			childrenVector.push_back(child.get());
			child->finalize();

			// In "or" nodes (or if no type is given), we keep the lowest value; in "and" nodes the highest one.
			node->setCost(maximize ? std::max(node->getCost(), child->getRoot()->getCost()) : std::min(node->getCost(), child->getRoot()->getCost()));
		}
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

		// We only print or count children with cost equal to this node's
		std::vector<ItemTree*> bestChildren;
		bestChildren.reserve(children.size());
		for(const auto& child : children)
			if(child->node->getCost() == node->getCost())
				bestChildren.push_back(child.get());
		assert(children.empty() || bestChildren.empty() == false);

		// When limiting the depth causes children not to be extended, print the number of accepting children (with optimum cost)
		if(maxDepth == 0 && children.empty() == false) {
			os << '[';
			mpz_class count;
			assert(count == 0);
			// On the first level we can use the counts inside the nodes
			// XXX This redundancy is a bit ugly but maybe offers better performance than recalculating the number of accepting children (like below in the "else" branch).
			if(!parent) {
				for(const auto& child : bestChildren)
					count += child->node->getCount();
			}
			else {
				for(const auto& child : bestChildren)
					count += child->node->countExtensions(*currentIt);
			}
			os << count << "] ";
		}

		for(const auto& item : items)
			os << item << ' ';

		// Print cost
		if(node->getCost() != 0 && (maxDepth == 0 || children.empty()))
			os << "(cost: " << node->getCost() << ')';

		os << std::endl;

		if(maxDepth > 0) {
			size_t i = 0;
			for(const auto& child : bestChildren)
				child->printExtensions(os, maxDepth - 1, false, ++i == bestChildren.size(), childIndent, currentIt.get());
		}
	}
}

void ItemTree::merge(const ItemTree& other)
{
	assert(node->getItems() == other.node->getItems());
	assert(node->getAuxItems() == other.node->getAuxItems());
	node->merge(std::move(*other.node));
	assert(children.size() == other.children.size());
	Children::const_iterator it = other.children.begin();
	for(const ItemTreePtr& child : children) {
		assert(it != other.children.end());
		child->merge(**it);
		++it;
	}
}
