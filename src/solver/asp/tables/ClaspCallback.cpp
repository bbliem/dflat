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

ClaspCallback::ClaspCallback(const ChildItemTrees& childItemTrees, const Application& app, const Clasp::ClaspFacade& clasp)
	: ::solver::asp::ClaspCallback(childItemTrees, app)
{
	for(const auto& pair : clasp.ctx.symTab()) {
		if(!pair.second.name.empty()) {
			const std::string name = pair.second.name.c_str();
			if(name.compare(0, 5, "item(") == 0) {
				const std::string argument = name.substr(5, name.length()-6);
				itemAtomInfos.emplace_back(ItemAtomInfo{{argument}, pair.second.lit});
			}
			else if(name.compare(0, 8, "auxItem(") == 0) {
				const std::string argument = name.substr(8, name.length()-9);
				auxItemAtomInfos.emplace_back(AuxItemAtomInfo{{argument}, pair.second.lit});
			}
			else if(name.compare(0, 7, "extend(") == 0) {
				const std::string argument = name.substr(7, name.length()-8);
				// Child node number is before the first '_' (and after the leading 'n')
				// Row number is after the first '_'
				const unsigned int underscorePos = argument.find('_');
				const unsigned int decompositionChildId = std::stoi(std::string(argument, 1, underscorePos-1));
				const unsigned int rowNumber = std::stoi(std::string(argument, underscorePos + 1));
				ASP_CHECK(childItemTrees.find(decompositionChildId) != childItemTrees.end(), "Extension pointer refers to nonexistent decomposition child");
				ASP_CHECK(rowNumber < childItemTrees.at(decompositionChildId)->getChildren().size(), "Extension pointer references invalid row number");
				extendAtomInfos.emplace_back(ExtendAtomInfo{{decompositionChildId, childItemTrees.at(decompositionChildId)->getChild(rowNumber).getRoot()}, pair.second.lit});
			}
			else if(name.compare(0, 12, "currentCost(") == 0) {
				const std::string argument = name.substr(12, name.length()-13);
				currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(argument)}, pair.second.lit});
			}
			else if(name.compare(0, 5, "cost(") == 0) {
				const std::string argument = name.substr(5, name.length()-6);
				costAtomInfos.emplace_back(CostAtomInfo{{std::stol(argument)}, pair.second.lit});
			}
		}
	}
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	solver::asp::ClaspCallback::onModel(s, m);

	// Get items {{{
	ItemTreeNode::Items items;
	forEachTrue(m, itemAtomInfos, [&items](const ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items auxItems;
	forEachTrue(m, auxItemAtomInfos, [&auxItems](const AuxItemAtomArguments& arguments) {
			auxItems.insert(arguments.item);
	});

	ASP_CHECK(std::find_if(items.begin(), items.end(), [&auxItems](const std::string& item) {
				return auxItems.find(item) != auxItems.end();
	}) == items.end(), "Items and auxiliary items not disjoint");
	// }}}
	// Get extension pointers {{{
	ItemTreeNode::ExtensionPointerTuple extendedRows;
	ASP_CHECK(countTrue(m, extendAtomInfos) == childItemTrees.size(), "Not as many extension pointers as there are child item trees");
	forEachTrueLimited(m, extendAtomInfos, [&](const ExtendAtomArguments& arguments) {
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
	ASP_CHECK(countTrue(m, costAtomInfos) <= 1, "More than one true cost/1 atom");
	long cost = 0;
	forFirstTrue(m, costAtomInfos, [&cost](const CostAtomArguments& arguments) {
			cost = arguments.cost;
	});
	node->setCost(cost);
	// }}}
	// Set current cost {{{
	ASP_CHECK(countTrue(m, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
	ASP_CHECK(countTrue(m, currentCostAtomInfos) == 0 || countTrue(m, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
	long currentCost = 0;
	forFirstTrue(m, currentCostAtomInfos, [&currentCost](const CurrentCostAtomArguments& arguments) {
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
	return true;
}

}}} // namespace solver::asp::tables
