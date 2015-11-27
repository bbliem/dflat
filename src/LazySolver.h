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

#pragma once
//}}}
#include <list>

#include "Solver.h"

class LazySolver : public Solver
{
public:
	LazySolver(const Decomposition& decomposition, const Application& app, bool branchAndBound = true);

	virtual ItemTreePtr compute() override final;

	// While the solver is computing its item tree, other objects can get the item tree that has been constructed so far with this method.
	virtual const ItemTreePtr& getItemTree() const = 0;

protected:
	typedef ItemTree::Children::const_iterator Row;

	// Recursively print all solver invocation results
	void printAllResults() const;

	const ItemTreeNode::ExtensionPointerTuple& getCurrentRowCombination() const
	{
		return currentRowCombination;
	}

	virtual void setItemTree(ItemTreePtr&& itemTree) = 0;

	// Results in getItemTree()->getChildren().end() if merging occurred for the last added row candidate, otherwise yields the newest row
	virtual Row getNewestRow() const = 0;

	// Call this when the solver will not compute any more rows. It returns the resulting item tree (and calls finalize() on it).
	virtual ItemTreePtr finalize() = 0;

	// Prepare solving step for child rows referred to by rowIterators such that subsequent calls to nextRowCandidate() return rows resulting from that combination
	virtual void startSolvingForCurrentRowCombination() = 0;
	virtual bool endOfRowCandidates() const = 0;
	virtual void nextRowCandidate() = 0;
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

private:
	// Compute (via lazy solving) the next row having cost less than costBound
	Row nextRow(long costBound);

	bool loadFirstChildRowCombination(long costBound);
	bool loadNextChildRowCombination(long costBound);
	// Returns false if we can establish that there is a child table having no relevant child rows joinable with newRow
	bool nextExistingRowCombination(size_t incrementPos = 0);
	void initializeItemTree(ItemTreeNode::ExtensionPointerTuple&& rootExtensionPointers);

	ItemTreeNode::ExtensionPointerTuple currentRowCombination;
	std::list<LazySolver*> nonExhaustedChildSolvers;
	std::list<LazySolver*>::const_iterator nextChildSolverToCall; // points to elements of nonExhaustedChildSolvers
	bool branchAndBound;
};
