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

namespace solver { namespace asp { namespace tables {

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
			for(const auto& atom : gringoOutput.getCountAtomInfos())
				countAtomInfos.emplace_back(CountAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
				currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getCostAtomInfos())
				costAtomInfos.emplace_back(CostAtomInfo(atom, symTab));
		}
	}
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	solver::asp::ClaspCallback::event(s, e, f);

	if(e != Clasp::ClaspFacade::event_model)
		return;

	// Get items
	ItemTreeNode::Items items;
	forEachTrue(s, itemAtomInfos, [&items](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items consequentItems;
	forEachTrue(s, consequentItemAtomInfos, [&consequentItems](const GringoOutputProcessor::ConsequentItemAtomArguments& arguments) {
			consequentItems.insert(arguments.item);
	});

	// Get extension pointers
	ItemTreeNode::ExtensionPointerTuple extendedRows;
	forEachTrueLimited(s, extendAtomInfos, [&](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			extendedRows.emplace(arguments.decompositionNodeId, ItemTreeNode::ExtensionPointer(arguments.extendedRow));
			return extendedRows.size() != childItemTrees.size();
	});

	// Get cost
	long cost = 0;
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});

	// TODO currentCost / count

	if(!itemTree) {
		ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
		for(const auto& childItemTree : childItemTrees)
			rootExtensionPointers.emplace(childItemTree.first, childItemTree.second->getRoot());
		itemTree = ItemTreePtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}))));
	}

	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(consequentItems), {std::move(extendedRows)}));
	node->setCost(cost);
	itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
}

}}} // namespace solver::asp::tables
