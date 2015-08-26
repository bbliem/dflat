/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include "Solver.h"

namespace solver { namespace lazy_clasp {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const Application& app)
	: ::solver::clasp::ClaspCallback(app)
	, gringoOutput(gringoOutput)
{
}

void ClaspCallback::initializeItemTree(ItemTreeNode::ExtensionPointerTuple&& rootExtensionPointers)
{
	// Create item tree root if it doesn't exist yet {{{
	if(!itemTree) {
		itemTree = ItemTreePtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}, ItemTreeNode::Type::OR))));
		// Set cost to "infinity"
		itemTree->getNode()->setCost(std::numeric_limits<decltype(itemTree->getNode()->getCost())>::max());
	}
	// }}}
}

void ClaspCallback::setExtendedRows(ItemTreeNode::ExtensionPointerTuple&& e)
{
	extendedRows = std::move(e);
}

ItemTree::Children::const_iterator ClaspCallback::getNewestRow() const
{
	return newestRow;
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	solver::clasp::ClaspCallback::onModel(s, m);

	// Get items {{{
	ItemTreeNode::Items items;
	forEachTrue(m, itemAtomInfos, [&items](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items auxItems;
	forEachTrue(m, auxItemAtomInfos, [&auxItems](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			auxItems.insert(arguments.item);
	});

	ASP_CHECK(std::find_if(items.begin(), items.end(), [&auxItems](const String& item) {
				return auxItems.find(item) != auxItems.end();
	}) == items.end(), "Items and auxiliary items not disjoint");
	// }}}
	assert(itemTree);
	// Create item tree node {{{
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {extendedRows}));
	// }}}
	// Set cost {{{
	ASP_CHECK(countTrue(m, costAtomInfos) <= 1, "More than one true cost/1 atom");
	long cost = 0;
	forFirstTrue(m, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
			cost = arguments.cost;
	});
	node->setCost(cost);
	// }}}
	// Set current cost {{{
	ASP_CHECK(countTrue(m, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
	ASP_CHECK(countTrue(m, currentCostAtomInfos) == 0 || countTrue(m, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
	long currentCost = 0;
	forFirstTrue(m, currentCostAtomInfos, [&currentCost](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
			currentCost = arguments.currentCost;
	});
	node->setCurrentCost(currentCost);
	// }}}
	// Possibly update cost of root {{{
	itemTree->getNode()->setCost(std::min(itemTree->getNode()->getCost(), cost));
	// }}}
	// Add node to item tree {{{
	ItemTree::Children::const_iterator newChild = itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
	// }}}

	if(newChild != itemTree->getChildren().end())
		newestRow = newChild;
	return true;
}

void ClaspCallback::prepare(const Clasp::SymbolTable& symTab)
{
	// XXX Necessary to call this before each solving invocation? Otherwise we could dispense with clear()
	itemAtomInfos.clear();
	for(const auto& atom : gringoOutput.getItemAtomInfos())
		itemAtomInfos.emplace_back(ItemAtomInfo(atom, symTab));
	auxItemAtomInfos.clear();
	for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, symTab));
	currentCostAtomInfos.clear();
	for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, symTab));
	costAtomInfos.clear();
	for(const auto& atom : gringoOutput.getCostAtomInfos())
		costAtomInfos.emplace_back(CostAtomInfo(atom, symTab));
}

}} // namespace solver::lazy_clasp
