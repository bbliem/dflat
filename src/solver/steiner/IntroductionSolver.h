/*{{{
Copyright 2018, Bernhard Bliem
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
#include "Solver.h"

namespace solver { namespace steiner {

class IntroductionSolver : public Solver
{
public:
	IntroductionSolver(const Decomposition& decomposition, const Application& app, unsigned introduced, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full);

	void startSolvingForCurrentRowCombination() override;
	virtual void nextRowCandidate() override;

private:
	const Instance& inducedInstance;
	const Instance::WeightedAdjacencyMatrix& weights; // of the induced instance

	unsigned introducedIndex; // index of introduced vertex
	bool introducedIsTerminal; // true iff introduced vertex is a terminal vertex
	std::vector<unsigned> introducedEdges; // indices (w.r.t. this decomposition node's induced instance) of vertices adjacent to introducedIndex

	std::vector<bool> guessedEdges; // current guess of a subset of introducedEdges (we iterate through all subsets)
	// TODO vector<bool> is probably inefficient for this purpose. Better use, e.g., GMP?

	// Update guessedEdges to the next guess. Returns false iff this was impossible.
	bool nextGuess();

	// Set selectedEdges to information from current child row and introduce new vertex. Retain observed terminals.
	void extendChildRow();
};

}} // namespace solver::steiner
