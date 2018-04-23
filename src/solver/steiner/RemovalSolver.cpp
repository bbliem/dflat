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
//}}}
#include "RemovalSolver.h"

namespace solver { namespace steiner {

RemovalSolver::RemovalSolver(const Decomposition& decomposition, const Application& app, unsigned removed, BranchAndBoundLevel bbLevel)
	: Solver(decomposition, app, bbLevel)
	, removedIndex((*decomposition.getChildren().begin())->getNode().getInducedInstance().getIndexOf(removed))
{
	removedIsTerminal = (*decomposition.getChildren().begin())->getNode().getInducedInstance().isTerminal(removedIndex);
}

void RemovalSolver::startSolvingForCurrentRowCombination()
{
	Solver::startSolvingForCurrentRowCombination();
	extendChildRow();
}

void RemovalSolver::nextRowCandidate()
{
	Solver::nextRowCandidate();
	end = true;
}

void RemovalSolver::extendChildRow()
{
	assert(getCurrentRowCombination().size() == 1); // We're in a remove node
	const RowPtr& extendedRow = getCurrentRowCombination().front();
	const AdjacencyMatrix& oldMatrix = extendedRow->getSelectedEdges();
	const Row::VertexList& oldSelectedVertices = extendedRow->getSelectedVertices();
	assert(oldSelectedVertices.size() == oldMatrix.getNumRows());
	const AdjacencyMatrix& oldConnected = extendedRow->getConnectedViaSelectedEdges();
	const auto size = oldMatrix.getNumRows();

	if(removedIsTerminal && oldSelectedVertices[removedIndex] == false) {
		// May not extend rows where the removed terminal vertex is not part of the solution
		end = true;
		return;
	}

	selectedVertices.clear();
	selectedVertices.reserve(size-1);
	bool selectedVertexExists = false;
	for(unsigned i = 0; i < size; ++i) {
		if(i != removedIndex) {
			selectedVertices.push_back(oldSelectedVertices[i]);
			selectedVertexExists = selectedVertexExists || oldSelectedVertices[i];
		}
	}

	hasForgottenComponent = extendedRow->getHasForgottenComponent();
	// Did we just forget a component?
	// This is the case if the removed vertex was selected but has no
	// selected neighbors in the current bag.
	if(!hasForgottenComponent && oldSelectedVertices[removedIndex]) {
		hasForgottenComponent = true;
		for(unsigned i = 0; i < size; ++i) {
			if(i != removedIndex && oldConnected(removedIndex,i)) {
				hasForgottenComponent = false;
				break;
			}
		}
	}
	// If we have forgotten a component, there may not be any selected
	// edges (and thus selected vertices) anymore.
	if(hasForgottenComponent && selectedVertexExists) {
		end = true;
		return;
	}

	selectedEdges = AdjacencyMatrix::remove(oldMatrix, removedIndex);
	connectedViaSelectedEdges = AdjacencyMatrix::remove(oldConnected, removedIndex);

	// Set current cost
	const auto weights = (*decomposition.getChildren().begin())->getNode().getInducedInstance().getAdjacencyMatrix()[removedIndex];
	assert(weights.size() == size);
	currentCost = extendedRow->getCurrentCost();
	for(unsigned i = 0; i < size; ++i) {
		if(oldMatrix(removedIndex,i))
			currentCost -= weights[i];
	}
	intersectionCost = currentCost;
}

}} // namespace solver::steiner
