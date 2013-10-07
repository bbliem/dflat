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
			// TODO
//			foreach(const Tree::Children::value_type& child, tree.children) {
//				const ExtendArguments& predecessors = child.first.first;
//				const Row::Items& topLevelItems = child.first.second;
//
//				Row::ExtensionPointerTuple extensionPointers;
//				extensionPointers.reserve(predecessors.size());
//				foreach(const string& predecessor, predecessors) {
//					// Child table number is before, row number is after the first '_'
//					const unsigned int underscorePos = predecessor.find('_');
//					unsigned int tableNumber = boost::lexical_cast<unsigned int>(std::string(predecessor, 1, underscorePos-1)); // predecessor starts with 'r'
//					unsigned int rowNumber = boost::lexical_cast<unsigned int>(std::string(predecessor, underscorePos + 1));
//					// TODO: Instead of the following assertions, throw assertions if invalid extension pointers are given. Also, add a check that ensures that if extension pointers are given, exactly one is given for each child table.
//					assert(tableNumber < childTablesVec.size());
//					assert(rowNumber < childTablesVec[tableNumber].size());
//					extensionPointers.push_back(dynamic_cast<const Row*>(childTablesVec[tableNumber][rowNumber]));
//				}
//				Row* row = new Row(Row::Tree(topLevelItems, child.second.mergeChildren()), extensionPointers);
//
//				if(child.second.hasCount)
//					row->setCount(child.second.count);
//				if(child.second.hasCurrentCost)
//					row->setCurrentCost(child.second.currentCost);
//				if(child.second.hasCost)
//					row->setCost(child.second.cost);
//
//				algorithm.addRowToTable(table, row);
//			}
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
			numLevels = arguments.length;
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

	////

	long count = 0, cost = 0; // TODO currentCost

	forFirstTrue(s, countAtomInfos, [&count](const GringoOutputProcessor::CountAtomArguments& arguments) {
			count = arguments.count;
	});
	forFirstTrue(s, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});

	// Convert branchData to UncompressedItemTree::Branch
	UncompressedItemTree::Branch branch;
	for(BranchNode& node : branchData)
		branch.emplace_back(UncompressedItemTreePtr(new UncompressedItemTree(UncompressedItemTree::Node(new ItemTreeNode(std::move(node.items), {std::move(node.extended)}))))); // TODO cost etc.

	// Insert branch into tree
	// TODO make this an exception
	assert(!uncompressedItemTree || (uncompressedItemTree->getRoot()->getItems() == branch.front()->getRoot()->getItems() && uncompressedItemTree->getRoot()->getExtensionPointers() == branch.front()->getRoot()->getExtensionPointers()));

	if(!uncompressedItemTree)
		uncompressedItemTree = std::move(branch.front());

	uncompressedItemTree->addBranch(++branch.begin(), branch.end());
}

}}} // namespace solver::asp::trees
