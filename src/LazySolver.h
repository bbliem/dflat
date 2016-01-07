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

#pragma once
//}}}
#include <list>

#include "Solver.h"

class LazySolver : public Solver
{
public:
	enum class BranchAndBoundLevel {
		none,  // No branch and bound
		basic, // Prevent rows not cheaper than current provisional solution
		full   // Additionally take lower bounds for forgotten subgraphs into account
	};
	LazySolver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full);

	virtual ItemTreePtr compute() override final;

	// While the solver is computing its item tree, other objects can get the item tree that has been constructed so far with this method.
	const ItemTreePtr& getItemTree() const { return itemTree; }

protected:
	typedef ItemTree::Children::const_iterator Row;

	const ItemTreeNode::ExtensionPointerTuple& getCurrentRowCombination() const
	{
		return currentRowCombination;
	}

	// Prepare solving step for child rows referred to by rowIterators such that subsequent calls to nextRowCandidate() return rows resulting from that combination
	virtual void startSolvingForCurrentRowCombination() = 0;
	virtual bool endOfRowCandidates() const = 0;
	virtual void nextRowCandidate() = 0;
	// Sets newestRow to the newest row or itemTree->getChildren().end() if no new row was produced
	virtual void handleRowCandidate(long costBound) = 0;

	struct RowIterator
	{
		Row current;
		Row begin;
		Row end;
	};
	typedef std::vector<RowIterator> RowIterators;
	RowIterators rowIterators; // Key: Child node; Value: Row in the item tree at this child
	virtual bool resetRowIteratorsOnNewRow(Row newRow, const Decomposition& from);

	ItemTreePtr itemTree;
	// Equal to itemTree->getChildren().end() if merging occurred for the last added row candidate
	Row newestRow;

private:
	// Compute (via lazy solving) the next row having cost less than costBound
	Row nextRow(long costBound);

	bool loadFirstChildRowCombination(long costBound);
	bool loadNextChildRowCombination(long costBound);
	// Returns false if we can establish that there is a child table having no relevant child rows joinable with newRow
	bool nextExistingRowCombination(size_t incrementPos = 0);
	void initializeItemTrees();

	// Call this when the solver will not compute any more rows. It calls finalize() on the resulting item tree and calls Printer::solverInvocationResult.
	void finalize();

	// Recursively call finalize() (and thus print all solver invocation results)
	void finalizeRecursively();

	ItemTreeNode::ExtensionPointerTuple currentRowCombination;
	std::list<LazySolver*> nonExhaustedChildSolvers;
	std::list<LazySolver*>::const_iterator nextChildSolverToCall; // points to elements of nonExhaustedChildSolvers
	BranchAndBoundLevel bbLevel;
	bool finalized;

	// Currently known lower bound for the cost of a solution for the forgotten subgraph (0 until finalized)
	long forgottenCostLowerBound;
};
