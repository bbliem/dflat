/*{{{
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
//}}}
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

ItemTreeNode::Type UncompressedItemTree::prune()
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


ItemTreePtr UncompressedItemTree::compress()
{
	ItemTreePtr result(new ItemTree(std::move(node)));

	switch(result->getRoot()->getType()) {
		case ItemTreeNode::Type::OR:
			assert(result->getRoot()->getCost() == 0);
			assert(children.empty() == false);
			// Set cost to "infinity"
			result->getRoot()->setCost(std::numeric_limits<decltype(result->getRoot()->getCost())>::max());
			for(const auto& child : children) {
				ItemTreePtr compressedChild = child->compress();
				result->getRoot()->setCost(std::min(result->getRoot()->getCost(), compressedChild->getRoot()->getCost()));
				result->addChildAndMerge(std::move(compressedChild));
			}
			break;

		case ItemTreeNode::Type::AND:
			assert(result->getRoot()->getCost() == 0);
			assert(children.empty() == false);
			// Set cost to minus "infinity"
			result->getRoot()->setCost(std::numeric_limits<decltype(result->getRoot()->getCost())>::min());
			for(const auto& child : children) {
				ItemTreePtr compressedChild = child->compress();
				result->getRoot()->setCost(std::max(result->getRoot()->getCost(), compressedChild->getRoot()->getCost()));
				result->addChildAndMerge(std::move(compressedChild));
			}
			break;

		default:
			for(const auto& child : children)
				result->addChildAndMerge(child->compress());
			break;
	}

	return result;
}

}}} // namespace solver::asp::trees
