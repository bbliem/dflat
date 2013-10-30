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

#include "ClaspCallback.h"

namespace solver { namespace asp { namespace trees {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, bool printModels, bool prune)
	: ::solver::asp::ClaspCallback(childItemTrees, printModels, prune)
	, gringoOutput(gringoOutput)
{
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	::solver::asp::ClaspCallback::state(e, f);

	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			for(const auto& atom : gringoOutput.getItemAtomInfos())
				itemAtomInfos.emplace_back(ItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getConsequentItemAtomInfos())
				consequentItemAtomInfos.emplace_back(ConsequentItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getExtendAtomInfos())
				extendAtomInfos.emplace_back(ExtendAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
				currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCostAtomInfos())
				costAtomInfos.emplace_back(CostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getLengthAtomInfos())
				lengthAtomInfos.emplace_back(LengthAtomInfo(atom, symTab));
		}
		else if(e == Clasp::ClaspFacade::event_state_exit) {
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
		ItemTreeNode::Items consequentItems;
		ItemTreeNode::ExtensionPointerTuple extended;
	};

	// Get number of levels in the branch corresponding to this answer set
	ASP_CHECK(countTrue(s, lengthAtomInfos) == 0, "No true length/1 atom");
	ASP_CHECK(countTrue(s, lengthAtomInfos) > 1, "Multiple true length/1 atoms");
	unsigned int numLevels;
	forFirstTrue(s, lengthAtomInfos, [&numLevels](const GringoOutputProcessor::LengthAtomArguments& arguments) {
			numLevels = arguments.length+1;
	});
	std::vector<BranchNode> branchData(numLevels);

	// Get items
	forEachTrue(s, itemAtomInfos, [&branchData](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Item at level higher than branch length");
			branchData[arguments.level].items.insert(arguments.item);
	});
	forEachTrue(s, consequentItemAtomInfos, [&branchData](const GringoOutputProcessor::ConsequentItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Consequent item at level higher than branch length");
			branchData[arguments.level].consequentItems.insert(arguments.item);
	});

	// Get extension pointers
	forEachTrue(s, extendAtomInfos, [&](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Extension pointer at level higher than branch length");
			branchData[arguments.level].extended.emplace(arguments.decompositionNodeId, ItemTreeNode::ExtensionPointer(arguments.extendedNode));
	});

	// Checks on extension pointers and item sets
#ifndef DISABLE_ASP_CHECKS
	for(const BranchNode& node : branchData) {
		ASP_CHECK(node.extended.size() == childItemTrees.size(), "Not all extension pointer tuples within a branch have arity n, where n is the number of children in the decomposition");

		ASP_CHECK(std::find_if(node.items.begin(), node.items.end(), [&node](const std::string& item) {
				   return node.consequentItems.find(item) != node.consequentItems.end();
		}) == node.items.end(), "Items and consequent items not disjoint");
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
			 uncompressedItemTree->getRoot()->getConsequentItems() == branchData.front().consequentItems &&
			 uncompressedItemTree->getRoot()->getExtensionPointers().front() == branchData.front().extended),
			"Item tree branches specify different roots");
#endif

	// Convert branchData to UncompressedItemTree::Branch
	UncompressedItemTree::Branch branch;
	branch.reserve(numLevels);
	if(uncompressedItemTree)
		branch.emplace_back(uncompressedItemTree->getRoot());
	else
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData.front().items), std::move(branchData.front().consequentItems), {std::move(branchData.front().extended)})));

	for(size_t i = 1; i < branchData.size(); ++i)
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData[i].items), std::move(branchData[i].consequentItems), {std::move(branchData[i].extended)})));

	// Set (current) cost
	long cost = 0;
	ASP_CHECK(countTrue(s, costAtomInfos) <= 1, "More than one true cost/1 atom");
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});
	branch.back()->setCost(cost);
	long currentCost = 0;
	ASP_CHECK(countTrue(s, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
	ASP_CHECK(countTrue(s, currentCostAtomInfos) == 0 || countTrue(s, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
	forFirstTrue(s, currentCostAtomInfos, [&currentCost](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
			currentCost = arguments.currentCost;
	});
	branch.back()->setCurrentCost(currentCost);

	// Insert branch into tree
	if(!uncompressedItemTree)
		uncompressedItemTree = UncompressedItemTreePtr(new UncompressedItemTree(std::move(branch.front())));

	uncompressedItemTree->addBranch(++branch.begin(), branch.end());
}

ItemTreePtr ClaspCallback::finalize()
{
	// Prune the resulting tree
	if(prune && itemTree && itemTree->prune() == ItemTreeNode::Type::REJECT)
		itemTree.reset();

	return ::solver::asp::ClaspCallback::finalize();
}

}}} // namespace solver::asp::trees
