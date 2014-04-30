/*{{{
Copyright 2012-2014, Bernhard Bliem
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
//}}}
#include "ItemTree.h"
#include "Application.h"
#include "ExtensionIterator.h"

bool ItemTreePtrComparator::operator()(const ItemTreePtr& lhs, const ItemTreePtr& rhs)
{
	return lhs->getNode()->compareCostInsensitive(*rhs->getNode()) ||
		(!rhs->getNode()->compareCostInsensitive(*lhs->getNode()) &&
		 (std::lexicographical_compare(lhs->getChildren().begin(), lhs->getChildren().end(), rhs->getChildren().begin(), rhs->getChildren().end(), *this) ||
		  (!std::lexicographical_compare(rhs->getChildren().begin(), rhs->getChildren().end(), lhs->getChildren().begin(), lhs->getChildren().end(), *this) &&
		   lhs->costDifferenceSignIncrease(rhs))));
}

ItemTree::Children::const_iterator ItemTree::addChildAndMerge(ChildPtr&& subtree)
{
	assert(subtree);
	assert(subtree->getNode()->getParent() == nullptr);
#ifndef NDEBUG
	for(const auto& weakChild : node->getWeakChildren()) {
		assert(weakChild.expired() == false);
		assert(weakChild.lock().get() != subtree->getNode().get());
	}
#endif

	subtree->getNode()->setParent(node.get());
	std::pair<Children::iterator, bool> result = children.insert(std::move(subtree));
	// XXX If an equivalent element already exists in "children", it is unclear to me whether "subtree" is actually moved or not. (Maybe it depends on the implementation?)
	// For the time being, pray that it isn't moved in such a case.
	// http://stackoverflow.com/questions/10043716/stdunordered-settinsertt-is-argument-moved-if-it-exists

	if(!result.second) {
		// A subtree rooted at a child with all equal item sets already exists
		assert(subtree); // See remark above -- subtree was set to null if it was indeed moved...
		const ItemTreePtr& origChild = *result.first;

		// Unify subtree with origChild
		origChild->merge(std::move(*subtree));
		return children.end();
	}
	node->addWeakChild((*result.first)->getNode());
	return result.first;
}

bool ItemTree::finalize(const Application& app, bool pruneUndef, bool pruneRejecting)
{
	if(pruneUndef) {
		if(node->getType() == ItemTreeNode::Type::UNDEFINED)
			return false;
		pruneUndefined();
	}

	if(evaluate(pruneRejecting) == ItemTreeNode::Type::REJECT)
		return false;

	clearUnneededExtensionPointers(app);

	prepareChildrenRandomAccess();

	return true;
}

const ItemTree& ItemTree::getChild(size_t i) const
{
	assert(childrenVector.size() == children.size());
	assert(i < childrenVector.size());
	return *childrenVector[i];
}

void ItemTree::printExtensions(std::ostream& os, unsigned int maxDepth, bool printCount, bool root, bool lastChild, const std::string& indent, const ExtensionIterator* parent) const
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
				os << "\\- ";
				childIndent += "   ";
#endif
			}
			else {
#ifndef NO_UNICODE
				os << "┣━ ";
				childIndent += "┃  ";
#else
				os << "|- ";
				childIndent += "|  ";
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
			if(!printCount)
				os << ">=";
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
				child->printExtensions(os, maxDepth - 1, printCount, false, ++i == bestChildren.size(), childIndent, currentIt.get());
		}
	}
}

ItemTreeNode::Type ItemTree::evaluate(bool pruneRejecting)
{
	// UNDEFINED nodes always evaluate to UNDEFINED and no pruning is done for their descendants
	if(node->getType() == ItemTreeNode::Type::UNDEFINED)
		return ItemTreeNode::Type::UNDEFINED;

	// Prune children recursively
	bool allAccepting = true;
	bool allRejecting = true;

	Children::const_iterator it = children.begin();
	while(it != children.end()) {
		ItemTreeNode::Type childResult = (*it)->evaluate(pruneRejecting);
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
				if(pruneRejecting) {
					// Remove that child
					children.erase(it++);
				}
				else
					++it;
				break;
		}
	}

	// Determine acceptance status of this node, if possible
	switch(node->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
			assert(false); // returned above
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

void ItemTree::pruneUndefined()
{
	assert(node->getType() != ItemTreeNode::Type::UNDEFINED);
	assert((node->getType() != ItemTreeNode::Type::OR && node->getType() != ItemTreeNode::Type::AND) || !children.empty());
	Children::const_iterator it = children.begin();
	while(it != children.end()) {
		if((*it)->getNode()->getType() == ItemTreeNode::Type::UNDEFINED)
			children.erase(it++);
		else {
			(*it)->pruneUndefined();
			++it;
		}
	}
#ifndef DISABLE_CHECKS
	if(node->getType() == ItemTreeNode::Type::OR || node->getType() == ItemTreeNode::Type::AND)
		if(children.empty() && !node->getHasAcceptingChild() && !node->getHasRejectingChild()) // We pruned all children
			throw std::runtime_error("Inner item tree node with defined type had only undefined children");
#endif
}

void ItemTree::clearUnneededExtensionPointers(const Application& app, unsigned int currentDepth)
{
	if(app.isCountingDisabled()) {
		++currentDepth;
		if(currentDepth > app.getMaterializationDepth())
			for(const auto& child : children)
				child->getNode()->clearExtensionPointers();
	}
	else {
		if(currentDepth > app.getMaterializationDepth())
			for(const auto& child : children)
				child->getNode()->clearExtensionPointers();
		++currentDepth;
	}

	for(const auto& child : children)
		child->clearUnneededExtensionPointers(app, currentDepth);
}


bool ItemTree::costDifferenceSignIncrease(const ItemTreePtr& other) const
{
	assert(node->getItems() == other->node->getItems());
	assert(children.size() == other->children.size());

	if(children.size() < 2)
		return false;

	Children::const_iterator it1 = children.begin();
	Children::const_iterator it2 = other->children.begin();

	const int difference = (*it1)->getNode()->getCost() - (*it2)->getNode()->getCost(); // Actually we are only interested if this is greater, equal to, or smaller than 0

	while(++it1 != children.end()) {
		++it2;
		assert(it2 != other->children.end());
		const auto cost1 = (*it1)->getNode()->getCost();
		const auto cost2 = (*it2)->getNode()->getCost();

		if(cost1 < cost2) {
			if(difference >= 0)
				return false; // but other->costDifferenceSignIncrease(this) will hold
		}
		else if(cost1 == cost2) {
			if(difference < 0)
				return true;
			if(difference > 0)
				return false; // but other->costDifferenceSignIncrease(this) will hold
		}
		else if(difference <= 0)
			return true;

		if((*it1)->costDifferenceSignIncrease(*it2))
			return true;
	}
	return false;
}

void ItemTree::merge(ItemTree&& other)
{
	assert(node->getItems() == other.node->getItems());
	assert(node->getAuxItems() == other.node->getAuxItems());
	assert(node->getType() == other.node->getType());
	assert(node->getParent());
	assert(node->getParent() == other.node->getParent());

	// If the other node is better, throw away this node's data and retain the other one's.
	// If this node is better, do nothing (the other node is thrown away anyway).
	switch(node->getParent()->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
			break;

		case ItemTreeNode::Type::OR:
			if(other.getNode()->getCost() < node->getCost()) {
				*this = std::move(other);
				return;
			}
			else if(other.getNode()->getCost() > node->getCost())
				return;
			break;

		case ItemTreeNode::Type::AND:
			if(other.getNode()->getCost() > node->getCost()) {
				*this = std::move(other);
				return;
			}
			else if(other.getNode()->getCost() < node->getCost())
				return;
			break;

		default:
			assert(false);
	}

	// Inform other.node's children that this->node will be their new parent
	for(const ItemTreePtr& child : other.children)
		child->getNode()->setParent(node.get());

	node->merge(std::move(*other.node));

	assert(children.size() == other.children.size());
	Children::const_iterator it = other.children.begin();
	for(const ItemTreePtr& subtree : children) {
		assert(it != other.children.end());
		subtree->merge(std::move(**it));
		++it;
	}
}

void ItemTree::prepareChildrenRandomAccess()
{
	// Fill children vector for random access to children
	assert(childrenVector.empty());
	childrenVector.reserve(children.size());
	for(const auto& child : children) {
		childrenVector.push_back(child.get());
		child->prepareChildrenRandomAccess();
	}
}
