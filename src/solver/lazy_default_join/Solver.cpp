/*{{{
Copyright 2012-2016, Bernhard Bliem
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

//unsigned Solver::joinSetups = 0;
//unsigned Solver::joinCalls = 0;
//unsigned Solver::discardedJoinResults = 0;

Solver::Solver(const Decomposition& decomposition, const Application& app, bool setLeavesToAccept, BranchAndBoundLevel bbLevel, bool binarySearch)
	: ::LazySolver(decomposition, app, bbLevel)
	, binarySearch(binarySearch)
	, rowType(setLeavesToAccept ? ItemTreeNode::Type::ACCEPT : ItemTreeNode::Type::UNDEFINED)
	, currentRowCombinationExhausted(true)
{
	assert(decomposition.getChildren().size() > 1);
}

void Solver::startSolvingForCurrentRowCombination()
{
//	++joinSetups;
	assert(getCurrentRowCombination().empty() == false);
	assert(currentRowCombinationExhausted);
	if(binarySearch) {
		// Binary search in resetRowIteratorsOnNewRow() ensures that the current row combination is joinable
		assert(std::all_of(getCurrentRowCombination().begin()+1, getCurrentRowCombination().end(), [this](const ItemTreeNode::ExtensionPointer& node) {
					return node->getItems() == this->getCurrentRowCombination()[0]->getItems();
					}));
		currentRowCombinationExhausted = false;
	}
	else {
		const auto& extended = getCurrentRowCombination();
		const auto& items = extended[0]->getItems();
		currentRowCombinationExhausted = !std::all_of(extended.begin()+1, extended.end(), [&items](const ItemTreeNode::ExtensionPointer& node) {
				return node->getItems() == items;
				});
	}
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
//	++joinCalls;
	const auto& extended = getCurrentRowCombination();
	assert(extended.empty() == false);

	ItemTreeNode::Items items = (*extended.begin())->getItems();
	assert(std::all_of(extended.begin(), extended.end(), [&items](const ItemTreeNode::ExtensionPointer& node) {
			return node->getItems() == items;
	}));

	ItemTreeNode::Items auxItems;
	for(const auto& node : extended)
		auxItems.insert(node->getAuxItems().begin(), node->getAuxItems().end());

	// FIXME Do proper cost computations, not this item-set cardinality proof of concept
	long cost = items.size();
	for(const auto& node : extended)
		cost += node->getCost() - node->getItems().size();

	if(cost >= costBound) {
//		++discardedJoinResults;
		newestRow = itemTree->getChildren().end();
		return;
	}

	assert(itemTree);

	// Create item tree node
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {extended}, rowType));

	if(!app.isOptimizationDisabled()) {
		node->setCost(cost);
		node->setCurrentCost(node->getItems().size());

		// Possibly update cost of root
		itemTree->getNode()->setCost(std::min(itemTree->getNode()->getCost(), cost));
	}

	// Add node to item tree
	newestRow = itemTree->costChangeAfterAddChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
}

bool Solver::resetRowIteratorsOnNewRow(Row newRow, const Decomposition& from)
{
	if(!binarySearch)
		return LazySolver::resetRowIteratorsOnNewRow(newRow, from);

	rowIterators.clear();
	for(const auto& child : decomposition.getChildren()) {
		if(child.get() == &from) {
			Row end = newRow;
			++end;
			rowIterators.push_back({newRow, newRow, end});
		} else {
			const auto& rows = static_cast<LazySolver&>(child->getSolver()).getItemTree()->getChildren();
			assert(rows.begin() != rows.end());
			const auto range = std::equal_range(rows.begin(), rows.end(), *newRow,
					[](const ItemTreePtr& a, const ItemTreePtr& b) {
					return a->getNode()->getItems() < b->getNode()->getItems();
					});
			if(range.first == range.second)
				return false;
			rowIterators.push_back({range.first, range.first, range.second});
		}
	}

	return true;
}

}} // namespace solver::lazy_default_join
