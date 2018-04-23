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

class RemovalSolver : public Solver
{
public:
	RemovalSolver(const Decomposition& decomposition, const Application& app, unsigned removed, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full);

	virtual void startSolvingForCurrentRowCombination() override;
	virtual void nextRowCandidate() override;

private:
	unsigned removedIndex;
	bool removedIsTerminal; // true iff removed vertex is a terminal vertex

	// Set selectedEdges to information from current child row and remove the vertex. Retain observed terminals except for the removed vertex.
	void extendChildRow();
};

}} // namespace solver::steiner
