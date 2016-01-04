/*{{{
Copyright 2012-2016, Bernhard Bliem
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

#ifndef NDEBUG
#include <iostream>
#endif

bool UncompressedItemTreePtrComparator::operator()(const UncompressedItemTreePtr& lhs, const UncompressedItemTreePtr& rhs)
{
	return lhs->getNode()->getItems() < rhs->getNode()->getItems() ||
		(lhs->getNode()->getItems() == rhs->getNode()->getItems() &&
		 (lhs->getNode()->getType() < rhs->getNode()->getType() ||
		  (lhs->getNode()->getType() == rhs->getNode()->getType() &&
		   (lhs->getNode()->getHasAcceptingChild() < rhs->getNode()->getHasAcceptingChild() ||
		    (lhs->getNode()->getHasAcceptingChild() == rhs->getNode()->getHasAcceptingChild() &&
		     (lhs->getNode()->getHasRejectingChild() < rhs->getNode()->getHasRejectingChild() ||
		      (lhs->getNode()->getHasRejectingChild() == rhs->getNode()->getHasRejectingChild() &&
		       (lhs->getNode()->getAuxItems() < rhs->getNode()->getAuxItems() ||
		        (lhs->getNode()->getAuxItems() == rhs->getNode()->getAuxItems() &&
		         lhs->getNode()->getExtensionPointers() < rhs->getNode()->getExtensionPointers())))))))));
}

void UncompressedItemTree::addBranch(Branch::iterator begin, Branch::iterator end)
{
	if(begin != end) {
		std::pair<Children::iterator, bool> result = children.insert(UncompressedItemTreePtr(new UncompressedItemTree(std::move(*begin))));
		(*result.first)->addBranch(++begin, end);
	}
}

ItemTreePtr UncompressedItemTree::compress(bool ignoreUndefCost)
{
	ItemTreePtr result(new ItemTree(std::move(node)));

	switch(result->getNode()->getType()) {
		case ItemTreeNode::Type::OR:
			assert(result->getNode()->getCost() == 0);
			assert(children.empty() == false);
			// Set cost to "infinity"
			result->getNode()->setCost(std::numeric_limits<decltype(result->getNode()->getCost())>::max());
			for(const auto& child : children) {
				ItemTreePtr compressedChild = child->compress(ignoreUndefCost);
				if(!ignoreUndefCost || compressedChild->getNode()->getType() != ItemTreeNode::Type::UNDEFINED)
					result->getNode()->setCost(std::min(result->getNode()->getCost(), compressedChild->getNode()->getCost()));
				result->addChildAndMerge(std::move(compressedChild));
			}
			break;

		case ItemTreeNode::Type::AND:
			assert(result->getNode()->getCost() == 0);
			assert(children.empty() == false);
			// Set cost to minus "infinity"
			result->getNode()->setCost(std::numeric_limits<decltype(result->getNode()->getCost())>::min());
			for(const auto& child : children) {
				ItemTreePtr compressedChild = child->compress(ignoreUndefCost);
				if(!ignoreUndefCost || compressedChild->getNode()->getType() != ItemTreeNode::Type::UNDEFINED)
					result->getNode()->setCost(std::max(result->getNode()->getCost(), compressedChild->getNode()->getCost()));
				result->addChildAndMerge(std::move(compressedChild));
			}
			break;

		default:
			for(const auto& child : children)
				result->addChildAndMerge(child->compress(ignoreUndefCost));
			break;
	}

	return result;
}

#ifndef NDEBUG
void UncompressedItemTree::printDebug() const
{
	print(std::cout);
}
#endif
