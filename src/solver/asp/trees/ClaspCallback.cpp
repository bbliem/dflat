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
#include "ClaspCallback.h"

namespace solver { namespace asp { namespace trees {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, bool prune, const Debugger& debugger)
	: ::solver::asp::ClaspCallback(childItemTrees, debugger)
	, gringoOutput(gringoOutput)
	, prune(prune)
{
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			for(const auto& atom : gringoOutput.getItemAtomInfos())
				itemAtomInfos.emplace_back(ItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
				auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getExtendAtomInfos())
				extendAtomInfos.emplace_back(ExtendAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
				currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCostAtomInfos())
				costAtomInfos.emplace_back(CostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getLengthAtomInfos())
				lengthAtomInfos.emplace_back(LengthAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getOrAtomInfos())
				orAtomInfos.emplace_back(OrAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getAndAtomInfos())
				andAtomInfos.emplace_back(AndAtomInfo(atom, symTab));

			if(gringoOutput.getAcceptAtomKey())
				acceptLiteral.reset(new Clasp::Literal(symTab[*gringoOutput.getAcceptAtomKey()].lit));
			if(gringoOutput.getRejectAtomKey())
				rejectLiteral.reset(new Clasp::Literal(symTab[*gringoOutput.getRejectAtomKey()].lit));
		}
		else if(e == Clasp::ClaspFacade::event_state_exit) {
			if(prune && uncompressedItemTree && uncompressedItemTree->prune() == ItemTreeNode::Type::REJECT)
				uncompressedItemTree.reset();
			if(uncompressedItemTree)
				itemTree = uncompressedItemTree->compress();
		}
	}
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	solver::asp::ClaspCallback::event(s, e, f);

	if(e != Clasp::ClaspFacade::event_model)
		return;

	struct BranchNode
	{
		ItemTreeNode::Items items;
		ItemTreeNode::Items auxItems;
		ItemTreeNode::ExtensionPointerTuple extended;
		ItemTreeNode::Type type = ItemTreeNode::Type::UNDEFINED;
	};

	// Get number of levels in the branch corresponding to this answer set {{{
	ASP_CHECK(countTrue(s, lengthAtomInfos) != 0, "No true length/1 atom");
	ASP_CHECK(countTrue(s, lengthAtomInfos) <= 1, "Multiple true length/1 atoms");
	unsigned int numLevels = 0;
	forFirstTrue(s, lengthAtomInfos, [&numLevels](const GringoOutputProcessor::LengthAtomArguments& arguments) {
			numLevels = arguments.length+1;
	});
	assert(numLevels > 0);
	std::vector<BranchNode> branchData(numLevels);
	// }}}
	// Get items {{{
	forEachTrue(s, itemAtomInfos, [&branchData](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Item at level higher than branch length");
			branchData[arguments.level].items.insert(arguments.item);
	});
	forEachTrue(s, auxItemAtomInfos, [&branchData](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Auxiliary item at level higher than branch length");
			branchData[arguments.level].auxItems.insert(arguments.item);
	});
	// }}}
	// Get extension pointers {{{
	forEachTrue(s, extendAtomInfos, [&branchData](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Extension pointer at level higher than branch length");
			branchData[arguments.level].extended.emplace(arguments.decompositionNodeId, ItemTreeNode::ExtensionPointer(arguments.extendedNode));
	});
	// }}}
	// Checks on extension pointers and item sets {{{
#ifndef DISABLE_CHECKS
	for(const BranchNode& node : branchData) {
		ASP_CHECK(node.extended.size() == childItemTrees.size(), "Not all extension pointer tuples within a branch have arity n, where n is the number of children in the decomposition");

		ASP_CHECK(std::find_if(node.items.begin(), node.items.end(), [&node](const std::string& item) {
				   return node.auxItems.find(item) != node.auxItems.end();
		}) == node.items.end(), "Items and auxiliary items not disjoint");
	}

	for(const auto& pair : branchData[0].extended)
		ASP_CHECK(pair.second->getParent() == nullptr, "Level 0 extension pointer does not point to the root of an item tree");

	auto curNode = branchData.begin();
	auto prevNode = curNode++;
	while(curNode != branchData.end()) {
		ASP_CHECK(std::all_of(curNode->extended.begin(), curNode->extended.end(), [&prevNode](const ItemTreeNode::ExtensionPointerTuple::value_type& pair) {
				return pair.second->getParent() == prevNode->extended.at(pair.first).get();
		}), "Extension pointer at level n+1 does not point to a child of the extended level-n node");
		++prevNode;
		++curNode;
	}

	assert(!uncompressedItemTree || uncompressedItemTree->getRoot()->getExtensionPointers().size() == 1);
	ASP_CHECK(!uncompressedItemTree ||
			(uncompressedItemTree->getRoot()->getItems() == branchData.front().items &&
			 uncompressedItemTree->getRoot()->getAuxItems() == branchData.front().auxItems &&
			 uncompressedItemTree->getRoot()->getExtensionPointers().front() == branchData.front().extended),
			"Item tree branches specify different roots");
#endif
	// }}}
	// Set item tree node types (or, and, accept or reject) {{{
	forEachTrue(s, orAtomInfos, [&branchData](const GringoOutputProcessor::OrAtomArguments& arguments) {
			ASP_CHECK(arguments.level + 1 < branchData.size(), "Item tree node type specificiation 'or' at level higher than or equal to branch length");
			ASP_CHECK(branchData[arguments.level].type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree node");
			branchData[arguments.level].type = ItemTreeNode::Type::OR;
	});
	forEachTrue(s, andAtomInfos, [&branchData](const GringoOutputProcessor::AndAtomArguments& arguments) {
			ASP_CHECK(arguments.level + 1 < branchData.size(), "Item tree node type specificiation 'and' at level higher than or equal to branch length");
			ASP_CHECK(branchData[arguments.level].type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree node");
			branchData[arguments.level].type = ItemTreeNode::Type::AND;
	});
	if(acceptLiteral && s.isTrue(*acceptLiteral)) {
		ASP_CHECK(branchData.back().type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree leaf");
		branchData.back().type = ItemTreeNode::Type::ACCEPT;
	}
	if(rejectLiteral && s.isTrue(*rejectLiteral)) {
		ASP_CHECK(branchData.back().type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree leaf");
		branchData.back().type = ItemTreeNode::Type::REJECT;
	}
	// }}}
	// Convert branchData to UncompressedItemTree::Branch {{{
	UncompressedItemTree::Branch branch;
	branch.reserve(numLevels);
	if(uncompressedItemTree)
		branch.emplace_back(uncompressedItemTree->getRoot());
	else
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData.front().items), std::move(branchData.front().auxItems), {std::move(branchData.front().extended)}, branchData.front().type)));

	for(size_t i = 1; i < branchData.size(); ++i)
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData[i].items), std::move(branchData[i].auxItems), {std::move(branchData[i].extended)}, branchData[i].type)));
	// }}}
	// Set cost {{{
	ASP_CHECK(countTrue(s, costAtomInfos) <= 1, "More than one true cost/1 atom");
	ASP_CHECK(countTrue(s, costAtomInfos) == 0 || std::all_of(branchData.begin(), branchData.end()-1, [](const BranchNode& node) {
			return node.type != ItemTreeNode::Type::UNDEFINED;
	}), "Cost specified but not all types of (non-leaf) nodes are defined");
	long cost = 0;
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});
	branch.back()->setCost(cost);
	// }}}
	// Set current cost {{{
	ASP_CHECK(countTrue(s, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
	ASP_CHECK(countTrue(s, currentCostAtomInfos) == 0 || countTrue(s, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
	long currentCost = 0;
	forFirstTrue(s, currentCostAtomInfos, [&currentCost](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
			currentCost = arguments.currentCost;
	});
	branch.back()->setCurrentCost(currentCost);
	// }}}
	// Insert branch into tree {{{
	if(!uncompressedItemTree)
		uncompressedItemTree = UncompressedItemTreePtr(new UncompressedItemTree(std::move(branch.front())));

	uncompressedItemTree->addBranch(++branch.begin(), branch.end());
	// }}}
}

}}} // namespace solver::asp::trees
