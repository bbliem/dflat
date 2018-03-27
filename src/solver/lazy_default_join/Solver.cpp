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

Solver::Solver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel, bool binarySearch)
	: ::LazySolver(decomposition, app, bbLevel)
	, binarySearch(binarySearch)
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
		assert(std::all_of(getCurrentRowCombination().begin()+1, getCurrentRowCombination().end(), [this](const Row::ExtensionPointer& row) {
					return row->getItems() == this->getCurrentRowCombination()[0]->getItems();
					}));
		currentRowCombinationExhausted = false;
	}
	else {
		const auto& extended = getCurrentRowCombination();
		const auto& items = extended[0]->getItems();
		currentRowCombinationExhausted = !std::all_of(extended.begin()+1, extended.end(), [&items](const Row::ExtensionPointer& row) {
				return row->getItems() == items;
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

	Row::Items items = (*extended.begin())->getItems();
	assert(std::all_of(extended.begin(), extended.end(), [&items](const Row::ExtensionPointer& row) {
			return row->getItems() == items;
	}));

	Row::Items auxItems;
	for(const auto& row : extended)
		auxItems.insert(row->getAuxItems().begin(), row->getAuxItems().end());

	assert(table);

	// Create table row
	std::shared_ptr<Row> row(new Row(std::move(items), std::move(auxItems), {extended}));

	long cost = 0;
	long currentCost = 0;

	if(cost >= costBound) {
//		++discardedJoinResults;
		newestRow = table->getRows().end();
		return;
	}

	if(!app.isOptimizationDisabled()) {
		row->setCost(cost);
		row->setCurrentCost(currentCost);
	}

	// Add row to table
	newestRow = table->add(std::move(row));
}

bool Solver::resetRowIteratorsOnNewRow(Rows::const_iterator newRow, const Decomposition& from)
{
	if(!binarySearch)
		return LazySolver::resetRowIteratorsOnNewRow(newRow, from);

	rowIterators.clear();
	for(const auto& child : decomposition.getChildren()) {
		if(child.get() == &from) {
			Rows::const_iterator end = newRow;
			++end;
			rowIterators.push_back({newRow, newRow, end});
		} else {
			const auto& rows = static_cast<LazySolver&>(child->getSolver()).getTable()->getRows();
			assert(rows.begin() != rows.end());
			const auto range = std::equal_range(rows.begin(), rows.end(), *newRow,
					[](const RowPtr& a, const RowPtr& b) {
					return a->getItems() < b->getItems();
					});
			if(range.first == range.second)
				return false;
			rowIterators.push_back({range.first, range.first, range.second});
		}
	}

	return true;
}

}} // namespace solver::lazy_default_join
