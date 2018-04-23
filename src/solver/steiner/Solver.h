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

#include "../../Decomposition.h"
#include "../../LazySolver.h"
#include "../../AdjacencyMatrix.h"

namespace solver { namespace steiner {

class Solver : public ::LazySolver
{
public:
	Solver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full);

protected:
	virtual void startSolvingForCurrentRowCombination() override;
	virtual bool endOfRowCandidates() const override;
	virtual void nextRowCandidate() override;
	virtual void handleRowCandidate(unsigned long costBound) override;

	bool end = false;

	AdjacencyMatrix selectedEdges;
	Row::VertexList selectedVertices;
	AdjacencyMatrix connectedViaSelectedEdges;
	bool hasForgottenComponent;

	unsigned int currentCost; // sum of the cost of the intersections between the current row and its extended child rows
	unsigned int intersectionCost; // sum of the cost of the intersections between the current row and its extended child rows
};

}} // namespace solver::steiner
