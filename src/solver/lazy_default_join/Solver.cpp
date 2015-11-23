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
#include <algorithm>
#include <iostream>

#include "Solver.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace solver { namespace lazy_default_join {

Solver::Solver(const Decomposition& decomposition, const Application& app, bool setLeavesToAccept)
	: ::LazySolver(decomposition, app)
	, rowType(setLeavesToAccept ? ItemTreeNode::Type::ACCEPT : ItemTreeNode::Type::UNDEFINED)
	, currentRowCombinationExhausted(true)
{
	assert(decomposition.getChildren().size() > 1);
}

const ItemTreePtr& Solver::getItemTree() const
{
	return itemTree;
}

void Solver::setItemTree(ItemTreePtr&& it)
{
	itemTree = std::move(it);
}

ItemTree::Children::const_iterator Solver::getNewestRow() const
{
	return newestRow;
}

ItemTreePtr Solver::finalize()
{
	itemTree->finalize(app, false, false);
	return std::move(itemTree);
}

void Solver::startSolvingForCurrentRowCombination()
{
	assert(getCurrentRowCombination().empty() == false);
	assert(currentRowCombinationExhausted);
	currentRowCombinationExhausted = false;
	const auto& extended = getCurrentRowCombination();
	const auto& items = extended[0]->getItems();
	currentRowCombinationExhausted = !std::all_of(extended.begin()+1, extended.end(), [&items](const ItemTreeNode::ExtensionPointer& node) {
			return node->getItems() == items;
	});
}

bool Solver::endOfRowCandidates() const
{
	return currentRowCombinationExhausted;
}

void Solver::nextRowCandidate()
{
	currentRowCombinationExhausted = true;
}

void Solver::handleRowCandidate(long costBound)
{
	const auto& extended = getCurrentRowCombination();
	assert(extended.empty() == false);

	ItemTreeNode::Items items = (*extended.begin())->getItems();
	assert(std::all_of(extended.begin(), extended.end(), [&items](const ItemTreeNode::ExtensionPointer& node) {
				std::cout << "Foo: " << *node << '\n';
			return node->getItems() == items;
	}));

	ItemTreeNode::Items auxItems;
	for(const auto& node : extended)
		auxItems.insert(node->getAuxItems().begin(), node->getAuxItems().end());

	// FIXME Do proper cost computations, not this item-set cardinality proof of concept
	long cost = items.size();
	for(const auto& node : extended)
		cost += node->getCost() - node->getItems().size();

	if(cost < costBound) {
		assert(itemTree);

		// Create item tree node
		std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {extended}, rowType));
		node->setCost(cost);

		// Possibly update cost of root
		itemTree->getNode()->setCost(std::min(itemTree->getNode()->getCost(), cost));

		// Add node to item tree
		newestRow = itemTree->costChangeAfterAddChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
	}
}

//Solver::RowRange Solver::relevantRange(const ItemTree::Children& rows, const ItemTreePtr& newRow) const
//{
//	return std::equal_range(rows.begin(), rows.end(), newRow,
//			[](const ItemTreePtr& a, const ItemTreePtr& b) {
//			return a->getNode()->getItems() < b->getNode()->getItems();
//			});
//}

}} // namespace solver::lazy_default_join
