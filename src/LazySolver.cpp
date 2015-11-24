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
#include <sstream>

#include "Decomposition.h"
#include "Application.h"
#include "Printer.h"
#include "LazySolver.h"

LazySolver::LazySolver(const Decomposition& decomposition, const Application& app)
	: ::Solver(decomposition, app)
{
	for(const auto& child : decomposition.getChildren())
		nonExhaustedChildSolvers.push_back(static_cast<LazySolver*>(&child->getSolver()));
	nextChildSolverToCall = nonExhaustedChildSolvers.begin();
	currentRowCombination.reserve(decomposition.getChildren().size());
	rowIterators.reserve(decomposition.getChildren().size());
}

bool LazySolver::resetRowIteratorsOnNewRow(Row newRow, const Decomposition& from)
{
	rowIterators.clear();
	for(const auto& child : decomposition.getChildren()) {
		if(child.get() == &from) {
			Row end = newRow;
			++end;
			rowIterators.push_back({newRow, newRow, end});
		} else {
			const auto& rows = static_cast<LazySolver&>(child->getSolver()).getItemTree()->getChildren();
			assert(rows.begin() != rows.end());
			rowIterators.push_back({rows.begin(), rows.begin(), rows.end()});
		}
	}

	return true;
}


LazySolver::Row LazySolver::nextRow(long costBound)
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	if(!getItemTree()) {
		if(loadFirstChildRowCombination(costBound) == false)
			return getItemTree()->getChildren().end();
		startSolvingForCurrentRowCombination();
	}

	assert(getItemTree());

	do {
		while(endOfRowCandidates()) {
			if(loadNextChildRowCombination(costBound) == false)
				return getItemTree()->getChildren().end();
			startSolvingForCurrentRowCombination();
		}

		handleRowCandidate(costBound);

		if(app.getPrinter().listensForSolverEvents()) {
			std::ostringstream msg;
			msg << "Node " << decomposition.getNode().getGlobalId() << ": ";
			if(getNewestRow() == getItemTree()->getChildren().end() || (*getNewestRow())->getNode()->getCost() >= costBound)
				msg << "[no new or better row]";
			else
				msg << (*getNewestRow())->getNode();
			app.getPrinter().solverEvent(msg.str());
		}

		nextRowCandidate();
	} while(getNewestRow() == getItemTree()->getChildren().end() || (*getNewestRow())->getNode()->getCost() >= costBound);

	return getNewestRow();
}

bool LazySolver::loadFirstChildRowCombination(long costBound)
{
	assert(!getItemTree());
	assert(rowIterators.empty());

	// Get the first row from each child node
	LazySolver* solver = nullptr;
	Row newRow;
	for(const auto& child : decomposition.getChildren()) {
		solver = static_cast<LazySolver*>(&child->getSolver());
		assert(!solver->getItemTree());
		newRow = solver->nextRow(costBound);
		assert(newRow == solver->getItemTree()->getChildren().begin());
		if(newRow == solver->getItemTree()->getChildren().end())
			return false;
	}
	// Now (solver != nullptr and newRow is set) iff there are child nodes
	assert(solver || decomposition.getChildren().empty());
	assert(!solver || !decomposition.getChildren().empty());

	// Initialize resulting item tree by telling it the roots of the child item trees
	ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
	rootExtensionPointers.reserve(decomposition.getChildren().size());
	for(const auto& child : decomposition.getChildren())
		rootExtensionPointers.push_back(static_cast<LazySolver&>(child->getSolver()).getItemTree()->getNode());
	initializeItemTree(std::move(rootExtensionPointers));

	// Set row iterators.
	// If this fails, load next child row combination, which will in turn load
	// new child rows until row iterators can be set.
	if(solver &&
			resetRowIteratorsOnNewRow(newRow, solver->decomposition) == false &&
			loadNextChildRowCombination(costBound) == false)
		return false;

	assert(rowIterators.size() == decomposition.getChildren().size());
	currentRowCombination.clear();
	for(const auto& it : rowIterators)
		currentRowCombination.push_back((*it.current)->getNode());

	return true;
}

bool LazySolver::loadNextChildRowCombination(long costBound)
{
	if(decomposition.getChildren().empty())
		return false;

	// Try next combination of existing child rows
	if(nextExistingRowCombination() == false) {
		// There is no combination of existing child rows, so we compute a new one
		LazySolver* childSolver;
		Row newRow;
		do {
			assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());
			childSolver = *nextChildSolverToCall;
			newRow = childSolver->nextRow(costBound);

			while(newRow == childSolver->getItemTree()->getChildren().end()) {
				// The child solver is now exhausted
				// Remove it from nonExhaustedChildSolvers
				// Set nextChildSolverToCall to the next one
				nonExhaustedChildSolvers.erase(nextChildSolverToCall++);

				if(nonExhaustedChildSolvers.empty())
					return false;

				if(nextChildSolverToCall == nonExhaustedChildSolvers.end())
					nextChildSolverToCall = nonExhaustedChildSolvers.begin();

				assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());
				childSolver = *nextChildSolverToCall;
				newRow = childSolver->nextRow(costBound);
			}

			// Now we have computed a new child row
//			originOfLastChildRow = childSolver->decomposition.getNode().getGlobalId();
			++nextChildSolverToCall;
			if(nextChildSolverToCall == nonExhaustedChildSolvers.end())
				nextChildSolverToCall = nonExhaustedChildSolvers.begin();
			assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());
		} while(resetRowIteratorsOnNewRow(newRow, childSolver->decomposition) == false);
	}

	currentRowCombination.clear();
	for(const auto& it : rowIterators)
		currentRowCombination.push_back((*it.current)->getNode());

	return true;
}

bool LazySolver::nextExistingRowCombination(size_t incrementPos)
{
	// Increment the iterator at index incrementPos, then reset all iterators
	// before it to their beginning position.
	// This will not increment the origin of the child row computed last
	// because the respective RowIterator's "end" is just one position after
	// its "begin".
	// Motivation: This is the new row we want to combine with all existing
	// ones.
	if(incrementPos >= rowIterators.size())
		return false;

//	// Don't increment originOfLastChildRow since this is the new row we want to combine with all existing ones
//	if(rowIterators[incrementPos].first->getNode().getGlobalId() == originOfLastChildRow)
//		return nextExistingRowCombination(incrementPos+1);

//	if(++rowIterators[incrementPos].second == static_cast<LazySolver&>(rowIterators[incrementPos].first->getSolver()).getItemTree()->getChildren().end())
	if(++rowIterators[incrementPos].current == rowIterators[incrementPos].end)
		return nextExistingRowCombination(incrementPos+1);
	else {
		for(size_t i = 0; i < incrementPos; ++i) {
//			if(rowIterators[i].first->getNode().getGlobalId() != originOfLastChildRow)
//				rowIterators[i].second = static_cast<LazySolver&>(rowIterators[i].first->getSolver()).getItemTree()->getChildren().begin();
			rowIterators[i].current = rowIterators[i].begin;
		}
	}
	return true;
}

void LazySolver::initializeItemTree(ItemTreeNode::ExtensionPointerTuple&& rootExtensionPointers)
{
	// Create item tree root if it doesn't exist yet
	if(!getItemTree()) {
		ItemTreePtr itemTree(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}, ItemTreeNode::Type::OR))));
		// Set cost to "infinity"
		itemTree->getNode()->setCost(std::numeric_limits<decltype(itemTree->getNode()->getCost())>::max());
		setItemTree(std::move(itemTree));
	}
}

ItemTreePtr LazySolver::compute()
{
	// Currently this is only called at the root of the decomposition.
	assert(decomposition.isRoot());
	// TODO Make optimization disableable to let us stop after finding the first row
	//nextRow(std::numeric_limits<long>::max());
	Row row = nextRow(std::numeric_limits<long>::max());
	while(row != getItemTree()->getChildren().end()) {
		row = nextRow((*row)->getNode()->getCost());
	}

	ItemTreePtr result = finalize();
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}
