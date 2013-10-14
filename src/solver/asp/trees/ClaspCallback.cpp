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

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, bool printModels)
	: ::solver::asp::ClaspCallback(childItemTrees, printModels)
	, gringoOutput(gringoOutput)
{
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			for(const auto& atom : gringoOutput.getItemAtomInfos())
				itemAtomInfos.emplace_back(ItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getExtendAtomInfos())
				extendAtomInfos.emplace_back(ExtendAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCountAtomInfos())
				countAtomInfos.emplace_back(CountAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
				currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCostAtomInfos())
				costAtomInfos.emplace_back(CostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getLengthAtomInfos())
				lengthAtomInfos.emplace_back(LengthAtomInfo(atom, symTab));

			if(printModels)
				std::cerr << std::endl;
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

	// Get number of levels in the branch corresponding to this answer set
	unsigned int numLevels = 0;
	forFirstTrue(s, lengthAtomInfos, [&numLevels](const GringoOutputProcessor::LengthAtomArguments& arguments) {
			assert(numLevels == 0);
			numLevels = arguments.length+1;
	});
	assert(numLevels > 0);

	struct BranchNode
	{
		ItemTreeNode::ExtensionPointerTuple extended;
		ItemTreeNode::Items items;
	};
	std::vector<BranchNode> branchData(numLevels);
	for(BranchNode& node : branchData)
		node.extended.reserve(childItemTrees.size());

	// Get items
	forEachTrue(s, itemAtomInfos, [&branchData](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			assert(arguments.level < branchData.size());
			branchData[arguments.level].items.insert(arguments.item);
	});

	// Get extension pointers
	forEachTrue(s, extendAtomInfos, [&](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			assert(arguments.level < branchData.size());
			branchData[arguments.level].extended.emplace_back(ItemTreeNode::ExtensionPointer(arguments.extendedNode));
	});

	long count = 0;
	forFirstTrue(s, countAtomInfos, [&count](const GringoOutputProcessor::CountAtomArguments& arguments) {
			count = arguments.count;
	});

	long cost = 0;
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});

	// TODO currentCost

	// Convert branchData to UncompressedItemTree::Branch
	UncompressedItemTree::Branch branch;
	for(BranchNode& node : branchData)
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(node.items), {std::move(node.extended)}))); // TODO cost etc.

	// Insert branch into tree
	// TODO make this an exception
	assert(!uncompressedItemTree || (uncompressedItemTree->getRoot()->getItems() == branch.front()->getItems() && uncompressedItemTree->getRoot()->getExtensionPointers() == branch.front()->getExtensionPointers()));

	if(!uncompressedItemTree)
		uncompressedItemTree = UncompressedItemTreePtr(new UncompressedItemTree(std::move(branch.front())));

	uncompressedItemTree->addBranch(++branch.begin(), branch.end());
}

ItemTreePtr ClaspCallback::finalize()
{
	// Prune the resulting tree
	//if(itemTree && itemTree->prune() == ItemTreeNode::Type::REJECT)
	//	itemTree.reset();
	if(itemTree) {
		std::cout << "Before pruning:\n" << *itemTree << '\n';
		if(itemTree->prune() == ItemTreeNode::Type::REJECT)
			itemTree.reset();
	}

	return ::solver::asp::ClaspCallback::finalize();
}

}}} // namespace solver::asp::trees
