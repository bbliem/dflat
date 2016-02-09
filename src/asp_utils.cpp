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
#include "asp_utils.h"

namespace asp_utils {

void declareDecomposition(const Decomposition& decomposition, std::ostream& out)
{
	out << "% Decomposition facts" << std::endl;
	out << "currentNode(" << decomposition.getNode().getGlobalId() << ")." << std::endl;
	for(const auto& v : decomposition.getNode().getBag()) {
		out << "bag(" << decomposition.getNode().getGlobalId() << ',' << v << "). ";
		out << "current(" << v << ")." << std::endl;
	}

	out << "#const numChildNodes=" << decomposition.getChildren().size() << '.' << std::endl;
	if(decomposition.getChildren().empty())
		out << "initial." << std::endl;
	else {
		for(const auto& child : decomposition.getChildren()) {
			out << "childNode(" << child->getNode().getGlobalId() << ")." << std::endl;
			for(const auto& v : child->getNode().getBag()) {
				out << "bag(" << child->getNode().getGlobalId() << ',' << v << "). ";
				out << "-introduced(" << v << ")." << std::endl; // Redundant
			}
		}
	}

	if(decomposition.isRoot())
		out << "final." << std::endl;

	if(decomposition.isPostJoinNode())
		out << "postJoin." << std::endl;

	// Redundant predicates for convenience...
	out << "introduced(X) :- current(X), not -introduced(X)." << std::endl;
	out << "removed(X) :- childNode(N), bag(N,X), not current(X)." << std::endl;
}

void declareItemTree(std::ostream& out, const ItemTree* itemTree, bool tableMode, unsigned int nodeId, const std::string& itemSetName, const std::string& parent, unsigned int level)
{
	if(!itemTree)
		return;

	// Declare this item set
	if(tableMode) {
		if(parent.empty() == false)
			out << "childRow(" << itemSetName << ',' << nodeId << ")." << std::endl;
	} else {
		out << "atLevel(" << itemSetName << ',' << level << ")." << std::endl;
		out << "atNode(" << itemSetName << ',' << nodeId << ")." << std::endl;
		if(parent.empty()) {
			out << "root(" << itemSetName << ")." << std::endl;
			out << "rootOf(" << itemSetName << ',' << nodeId << ")." << std::endl;
		} else {
			out << "sub(" << parent << ',' << itemSetName << ")." << std::endl;
			if(itemTree->getChildren().empty()) {
				out << "leaf(" << itemSetName << ")." << std::endl;
				out << "leafOf(" << itemSetName << ',' << nodeId << ")." << std::endl;
			}
		}
	}
	for(const auto& item : itemTree->getNode()->getItems())
		out << "childItem(" << itemSetName << ',' << item << ")." << std::endl;
	for(const auto& item : itemTree->getNode()->getAuxItems())
		out << "childAuxItem(" << itemSetName << ',' << item << ")." << std::endl;

	// Declare item tree node type
	switch(itemTree->getNode()->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
			break;
		case ItemTreeNode::Type::OR:
			out << "childOr(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::AND:
			out << "childAnd(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::ACCEPT:
			out << "childAccept(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::REJECT:
			out << "childReject(" << itemSetName << ")." << std::endl;
			break;
	}

	// If this is a leaf, declare cost
	const ItemTree::Children& children = itemTree->getChildren();
	if(children.empty()) {
		out << "childCost(" << itemSetName << ',' << itemTree->getNode()->getCost() << ")." << std::endl;
	}
	else {
		// Declare child item sets
		size_t i = 0;
		for(const auto& child : children) {
			std::ostringstream childName;
			childName << itemSetName << '_' << i++;
			declareItemTree(out, child.get(), tableMode, nodeId, childName.str(), itemSetName, level+1);
		}
	}
}

void induceSubinstance(std::ostream& out, const Instance& instance, const DecompositionNode::Bag& bag)
{
	out << "% Induced subinstance" << std::endl;
	for(const auto& str : instance.getNonEdgeFacts())
		out << str << '.' << std::endl;

	// Select edge facts whose arguments are all in the bag
	for(auto facts : instance.getEdgeFacts()) {
		for(const Instance::Edge& edge : facts.second) {
			if(!std::any_of(edge.begin(), edge.end(), [&](String v) { return bag.find(v) == bag.end();})) {
				out << facts.first;
				char sep = '(';
				assert(edge.size() > 0);
				for(String v : edge) {
					out << sep << v;
					sep = ',';
				}
				out << ")." << std::endl;
			}
		}
	}
}

} // namespace asp_utils
