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
#include <clasp/clasp_facade.h>

#include "ClaspCallback.h"
#include "../../Decomposition.h"
#include "SolverIter.h"

namespace solver { namespace lazy_clasp {

class Solver : public ::Solver
{
public:
	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles);

	virtual ItemTreePtr compute() override;

	ItemTree::Children::const_iterator nextRow(long costBound);

	// When the solver is currently in this->compute(), other objects can get the item tree that has been constructed so far with this method.
	const ItemTreePtr& getItemTreeSoFar() const;

private:
	std::vector<std::string> encodingFiles;
	std::vector<Clasp::Var> variables;
	std::unordered_map<String, size_t> itemsToVarIndices;

	// Computes the first row for each child table, sets the clasp solving assumptions and starts asynchronous solving
	bool loadFirstChildRowCombination(long costBound);
	bool loadNextChildRowCombination(long costBound);
	void startSolvingForCurrentRowCombination();
	void resetRowIteratorsOnNewRow(ItemTree::Children::const_iterator newRow);
	bool nextExistingRowCombination(size_t incrementPos = 0);

	std::unique_ptr<ClaspCallback> claspCallback;
	std::unique_ptr<Gringo::Output::LparseOutputter> lpOut;

	Clasp::ClaspFacade clasp;
	Clasp::ClaspConfig config;
	std::unique_ptr<SolveIter> asyncResult;
	typedef std::pair<Decomposition*, ItemTree::Children::const_iterator> RowIteratorPair;
	typedef std::vector<RowIteratorPair> RowIterators;
	RowIterators rowIterators; // Key: Child node; Value: Row in the item tree at this child
	//int nextChildNodeToCall = 0;
	std::list<Solver*> nonExhaustedChildSolvers;
	std::list<Solver*>::const_iterator nextChildSolverToCall; // points to elements of nonExhaustedChildSolvers
	unsigned int originOfLastChildRow = -1; // value is the ID of the DecompositionNode where the last child row has been computed
};

}} // namespace solver::lazy_clasp
