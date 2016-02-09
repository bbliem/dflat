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
#include <sstream>
#include <algorithm>

#include "Decomposition.h"
#include "Application.h"
#include "Printer.h"
#include "LazySolver.h"

//// XXX remove
//#include "solver/lazy_clasp/Solver.h"
//#include "solver/lazy_default_join/Solver.h"

LazySolver::LazySolver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel)
	: ::Solver(decomposition, app)
	, bbLevel(bbLevel)
	, finalized(false)
	, forgottenCostLowerBound(0)
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
			const auto& rows = static_cast<LazySolver&>(child->getSolver()).itemTree->getChildren();
			assert(rows.begin() != rows.end());
			rowIterators.push_back({rows.begin(), rows.begin(), rows.end()});
		}
	}

	return true;
}


LazySolver::Row LazySolver::nextRow(long costBound)
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	if(itemTree->getChildren().empty()) {
		if(loadFirstChildRowCombination(costBound) == false) {
			finalize();
			return itemTree->getChildren().end();
		}
		startSolvingForCurrentRowCombination();
	}

	do {
		while(endOfRowCandidates()) {
			if(loadNextChildRowCombination(costBound) == false) {
				finalize();
				return itemTree->getChildren().end();
			}
			startSolvingForCurrentRowCombination();
		}

		handleRowCandidate(costBound);
		assert(newestRow == itemTree->getChildren().end() || (*newestRow)->getNode()->getCost() < costBound);

		if(app.getPrinter().listensForSolverEvents()) {
			std::ostringstream msg;
			msg << "Node " << decomposition.getNode().getGlobalId() << ": ";
			if(newestRow == itemTree->getChildren().end())
				msg << "[no new row within cost bound]";
			else
				msg << (*newestRow)->getNode();
			app.getPrinter().solverEvent(msg.str());
		}

		nextRowCandidate();
	} while(newestRow == itemTree->getChildren().end());

	return newestRow;
}

bool LazySolver::loadFirstChildRowCombination(long costBound)
{
	assert(itemTree->getChildren().empty());
	assert(rowIterators.empty());

	// Get the first row from each child node
	LazySolver* solver = nullptr;
	Row newRow;
	for(const auto& child : decomposition.getChildren()) {
		solver = static_cast<LazySolver*>(&child->getSolver());
		assert(solver->itemTree->getChildren().empty());

		assert(forgottenCostLowerBound >= solver->forgottenCostLowerBound);
		assert(bbLevel != BranchAndBoundLevel::none || forgottenCostLowerBound == 0);
		assert(bbLevel != BranchAndBoundLevel::none || solver->forgottenCostLowerBound == 0);
		forgottenCostLowerBound -= solver->forgottenCostLowerBound;
		newRow = solver->nextRow(costBound - forgottenCostLowerBound);
		forgottenCostLowerBound += solver->forgottenCostLowerBound;

		assert(newRow == solver->itemTree->getChildren().begin());
		if(newRow == solver->itemTree->getChildren().end())
			return false;
	}
	// Now (solver != nullptr and newRow is set) iff there are child nodes
	assert(solver || decomposition.getChildren().empty());
	assert(!solver || !decomposition.getChildren().empty());

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

			// For computing a row at this child, reduce cost bound by sum of lower bounds on costs for forgotten vertices
			assert(forgottenCostLowerBound >= childSolver->forgottenCostLowerBound);
			assert(bbLevel != BranchAndBoundLevel::none || forgottenCostLowerBound == 0);
			assert(bbLevel != BranchAndBoundLevel::none || childSolver->forgottenCostLowerBound == 0);
			forgottenCostLowerBound -= childSolver->forgottenCostLowerBound;
			newRow = childSolver->nextRow(costBound - forgottenCostLowerBound);
			forgottenCostLowerBound += childSolver->forgottenCostLowerBound;

			while(newRow == childSolver->itemTree->getChildren().end()) {
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

				assert(forgottenCostLowerBound >= childSolver->forgottenCostLowerBound);
				assert(bbLevel != BranchAndBoundLevel::none || forgottenCostLowerBound == 0);
				assert(bbLevel != BranchAndBoundLevel::none || childSolver->forgottenCostLowerBound == 0);
				forgottenCostLowerBound -= childSolver->forgottenCostLowerBound;
				newRow = childSolver->nextRow(costBound - forgottenCostLowerBound);
				forgottenCostLowerBound += childSolver->forgottenCostLowerBound;
			}

			// Now we have computed a new child row
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

	if(++rowIterators[incrementPos].current == rowIterators[incrementPos].end)
		return nextExistingRowCombination(incrementPos+1);
	else {
		for(size_t i = 0; i < incrementPos; ++i)
			rowIterators[i].current = rowIterators[i].begin;
	}
	return true;
}

void LazySolver::initializeItemTrees()
{
	assert(!itemTree);
	ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
	rootExtensionPointers.reserve(decomposition.getChildren().size());
	for(const auto& child : decomposition.getChildren()) {
		auto& solver = static_cast<LazySolver&>(child->getSolver());
		assert(!solver.itemTree);
		solver.initializeItemTrees();
		assert(solver.itemTree);
		rootExtensionPointers.push_back(solver.itemTree->getNode());
	}

	itemTree.reset(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}, ItemTreeNode::Type::OR))));

	// Set cost to "infinity"
	if(!app.isOptimizationDisabled())
		itemTree->getNode()->setCost(std::numeric_limits<decltype(itemTree->getNode()->getCost())>::max());
}

ItemTreePtr LazySolver::compute()
{
	// Currently this is only called at the root of the decomposition.
	assert(decomposition.isRoot());

	// Initialize item trees of all decomposition nodes
	initializeItemTrees();

	Row row = nextRow(std::numeric_limits<long>::max());

	// If we are solving an optimization problem, check optimality of "row" by
	// finding more solutions with cost of "row" as the bound
	if(!app.isOptimizationDisabled()) {
		while(row != itemTree->getChildren().end()) {
			if(app.getPrinter().listensForSolverEvents()) {
				std::ostringstream msg;
				msg << "Found new solution with cost " << (*row)->getNode()->getCost();
				app.getPrinter().solverEvent(msg.str());
			}
			app.getPrinter().provisionalSolution(*(*row)->getNode());

			const long newCostBound = bbLevel == BranchAndBoundLevel::none ? std::numeric_limits<long>::max() : (*row)->getNode()->getCost();
			row = nextRow(newCostBound);
		}
	}

//	std::cout << "ASP:\n"
//		<< "setups: " << solver::lazy_clasp::Solver::solverSetups << '\n'
//		<< "solve calls: " << solver::lazy_clasp::Solver::solverSetups << '\n'
//		<< "models: " << solver::lazy_clasp::Solver::models << '\n'
//		<< "discarded models: " << solver::lazy_clasp::Solver::discardedModels << '\n'
//		<< "Join:\n"
//		<< "setups: " << solver::lazy_default_join::Solver::joinSetups << '\n'
//		<< "join calls: " << solver::lazy_default_join::Solver::joinCalls << '\n'
//		<< "discarded join results: " << solver::lazy_default_join::Solver::discardedJoinResults << '\n';

	finalizeRecursively();
	return std::move(itemTree);
}

void LazySolver::finalize()
{
	assert(!finalized);
	if(itemTree && itemTree->finalize(app, false, false) == false)
		itemTree.reset();
	app.getPrinter().solverInvocationResult(decomposition, itemTree.get());
	finalized = true;

	// Calculate lower bound for the cost of a solution for the forgotten subgraph
	if(itemTree && bbLevel == BranchAndBoundLevel::full) {
		forgottenCostLowerBound = std::numeric_limits<long>::max();
		assert(itemTree->getChildren().empty() == false);

		for(const auto& row : itemTree->getChildren()) {
			const auto& node = *row->getNode();
			// FIXME We should eventually be able to deal with negative costs
			assert(node.getCost() >= 0);
			forgottenCostLowerBound = std::min(forgottenCostLowerBound, node.getCost() - node.getCurrentCost());
		}
	}
}

void LazySolver::finalizeRecursively()
{
	for(const auto& child : decomposition.getChildren())
		static_cast<LazySolver&>(child->getSolver()).finalizeRecursively();

	if(!finalized)
		finalize();
}
