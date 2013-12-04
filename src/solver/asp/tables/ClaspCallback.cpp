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

namespace solver { namespace asp { namespace tables {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, const Debugger& debugger)
	: ::solver::asp::ClaspCallback(childItemTrees, debugger)
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
			for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
				auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, symTab));
			for(const auto& atom : gringoOutput.getExtendAtomInfos())
				extendAtomInfos.emplace_back(ExtendAtomInfo(atom, symTab));
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

	// Get items {{{
	ItemTreeNode::Items items;
	forEachTrue(s, itemAtomInfos, [&items](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items auxItems;
	forEachTrue(s, auxItemAtomInfos, [&auxItems](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			auxItems.insert(arguments.item);
	});

	ASP_CHECK(std::find_if(items.begin(), items.end(), [&auxItems](const std::string& item) {
			   return auxItems.find(item) != auxItems.end();
	}) == items.end(), "Items and auxiliary items not disjoint");
	// }}}
	// Get extension pointers {{{
	ItemTreeNode::ExtensionPointerTuple extendedRows;
	ASP_CHECK(countTrue(s, extendAtomInfos) == childItemTrees.size(), "Not as many extension pointers as there are child item trees");
	forEachTrueLimited(s, extendAtomInfos, [&](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			extendedRows.emplace(arguments.decompositionNodeId, ItemTreeNode::ExtensionPointer(arguments.extendedRow));
			return extendedRows.size() != childItemTrees.size();
	});
	// }}}
	// Create item tree root if it doesn't exist yet {{{
	if(!itemTree) {
		ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
		for(const auto& childItemTree : childItemTrees)
			rootExtensionPointers.emplace(childItemTree.first, childItemTree.second->getRoot());
		itemTree = ItemTreePtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}, ItemTreeNode::Type::OR))));
		// Set cost to "infinity"
		itemTree->getRoot()->setCost(std::numeric_limits<decltype(itemTree->getRoot()->getCost())>::max());
	}
	// }}}
	// Create item tree node {{{
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {std::move(extendedRows)}));
	// }}}
	// Set cost {{{
	ASP_CHECK(countTrue(s, costAtomInfos) <= 1, "More than one true cost/1 atom");
	long cost = 0;
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});
	node->setCost(cost);
	// }}}
	// Set current cost {{{
	ASP_CHECK(countTrue(s, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
	ASP_CHECK(countTrue(s, currentCostAtomInfos) == 0 || countTrue(s, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
	long currentCost = 0;
	forFirstTrue(s, currentCostAtomInfos, [&currentCost](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
			currentCost = arguments.currentCost;
	});
	node->setCurrentCost(currentCost);
	// }}}
	// Possibly update cost of root {{{
	itemTree->getRoot()->setCost(std::min(itemTree->getRoot()->getCost(), cost));
	// }}}
	// Add node to item tree {{{
	itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
	// }}}
}

}}} // namespace solver::asp::tables
