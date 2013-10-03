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

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees)
	: ::solver::asp::ClaspCallback(childItemTrees)
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
#ifdef PRINT_MODELS
			std::cout << std::endl;
#endif
		}
	}
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifdef PRINT_MODELS
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated-1 << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

	// Get items
	ItemTreeNode::Items items;
	for(const auto& atom : itemAtomInfos)
		if(s.isTrue(atom.literal))
			items.insert(atom.arguments.item);

	// Get extension pointers
	ItemTreeNode::ExtensionPointerTuple extendedRows;
	extendedRows.reserve(childItemTrees.size());

	for(const auto& atom : extendAtomInfos) {
		if(s.isTrue(atom.literal)) {
			// TODO: Instead of the following assertions, throw assertions if invalid extension pointers are given. Also, add a check that ensures that if extension pointers are given, exactly one is given for each child table.
			assert(childItemTrees.find(atom.arguments.childId) != childItemTrees.end());
			assert(atom.arguments.rowNumber < childItemTrees.at(atom.arguments.childId)->getChildren().size());

			extendedRows.push_back(childItemTrees.at(atom.arguments.childId)->getChild(atom.arguments.rowNumber).getRoot());

#ifdef DISABLE_ANSWER_SET_CHECKS
			if(extendedRows.size() == childItemTrees.size())
				break;
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(extendedRows.size() > 0 && extendedRows.size() != childItemTrees.size())
		throw std::runtime_error("Number of extended rows non-zero and not equal to number of child nodes");
#endif

	// Get cost
	long cost = 0;

	for(const auto& atom : costAtomInfos) {
		if(s.isTrue(atom.literal)) {

#ifndef DISABLE_ANSWER_SET_CHECKS
			if(cost != 0)
				throw std::runtime_error("Multiple costs");
#endif

			cost = atom.arguments.cost;

#ifdef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	// TODO currentCost / count

	if(!itemTree)
		itemTree = ItemTreePtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode)));

	itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode(std::move(items), {std::move(extendedRows)}, cost)))));
}

}}} // namespace solver::asp::tables
