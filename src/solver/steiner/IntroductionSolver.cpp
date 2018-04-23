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
#include "../../Application.h"
#include "IntroductionSolver.h"

// XXX remove
#include <iostream>
#ifndef NDEBUG
#include "../../Printer.h"
#include <sstream>
#define LOG(STR) \
	if(app.getPrinter().listensForSolverEvents()) { \
		std::ostringstream logmsg; \
		logmsg << "Node " << decomposition.getNode().getGlobalId() << ": " << STR; \
		app.getPrinter().solverEvent(logmsg.str()); \
	}
#else
#define LOG(STR) ;
#endif

namespace solver { namespace steiner {

IntroductionSolver::IntroductionSolver(const Decomposition& decomposition, const Application& app, unsigned introduced, BranchAndBoundLevel bbLevel)
	: Solver(decomposition, app, bbLevel)
	, inducedInstance(decomposition.getNode().getInducedInstance())
	, weights(decomposition.getNode().getInducedInstance().getAdjacencyMatrix())
	, introducedIndex(decomposition.getNode().getInducedInstance().getIndexOf(introduced))
{
	introducedIsTerminal = inducedInstance.isTerminal(introducedIndex);

	// Get induced edges adjacent to the introduced vertex
	const auto& adj = weights[inducedInstance.getIndexOf(introduced)];
	for(unsigned i = 0; i < adj.size(); ++i) {
		const unsigned weight = adj[i];
		if(weight > 0) {
			LOG("By introducing " << introduced << " (index " << introducedIndex << ") we introduced an edge to " << inducedInstance.getVertexNames()[i] << " (index " << i << ')');
			introducedEdges.push_back(i);
		}
	}

	// Initial guess for subset of introduced edges: empty set
	//guessedEdges.resize(introducedEdges.size(), false); // will be set in startSolvingForCurrentRowCombination
	guessedEdges.resize(introducedEdges.size());
}

void IntroductionSolver::startSolvingForCurrentRowCombination()
{
	Solver::startSolvingForCurrentRowCombination();

	// TODO
	extendChildRow();
	LOG("Selected edges after extendChildRow: " << selectedEdges);

	// Reset guessed introduced edges to the empty set
	assert(guessedEdges.size() == introducedEdges.size());
	std::fill(guessedEdges.begin(), guessedEdges.end(), false);
}

void IntroductionSolver::nextRowCandidate()
{
	Solver::nextRowCandidate();

	if(nextGuess() == false) {
		end = true;
		LOG("  Could not increment guess");
		return;
	}

#ifndef NDEBUG
	std::ostringstream msg;
	for(unsigned i = 0; i < guessedEdges.size(); ++i) {
		//if(guessedEdges[i])
		//    msg << ' ' << introducedEdges[i];
		msg << ' ' << guessedEdges[i];
	}
	LOG("  Incremented guess to:" << msg.str());
#endif

	// Update our data structures according to the new guess
	// TODO instead of re-initializing all of selectedEdges, it would probably be more efficient to just reset the new row and column.
	extendChildRow();
	for(unsigned i = 0; i < guessedEdges.size(); ++i) {
		const bool guessed = guessedEdges[i];
		if(guessed) {
			const unsigned otherVertex = introducedEdges[i];
			// We have guessed the edge (introducedIndex, otherVertex)
			selectedEdges.set(introducedIndex, otherVertex);
			//const bool otherIsTerminal = inducedInstance.isTerminal(otherVertex);
			//selectedVertices[introducedIndex] = selectedVertices[introducedIndex] || introducedIsTerminal;
			//selectedVertices[otherVertex] = selectedVertices[otherVertex] || otherIsTerminal;
			selectedVertices[introducedIndex] = true;
			selectedVertices[otherVertex] = true;
			// XXX We need to compute the transitive closure of the guessed
			// edges. Does it pay off compared to a normalization variant where
			// we have only one edge introduction per node and thus do not need
			// to compute it?
			connectedViaSelectedEdges.set(introducedIndex, otherVertex);
			currentCost += weights[introducedIndex][otherVertex];
		}
	}
	connectedViaSelectedEdges.makeTransitive();
#ifndef NDEBUG
	LOG("  New selected edges:" << selectedEdges);
	msg.clear();
	for(bool b : selectedVertices)
		msg << b;
	LOG("  New selected vertices: " << msg.str());
	LOG("  New connections: " << connectedViaSelectedEdges);
#endif
}

void IntroductionSolver::extendChildRow()
{
	assert(getCurrentRowCombination().size() == 1); // We're in an introduce node
	const RowPtr& extendedRow = getCurrentRowCombination().front();
	const AdjacencyMatrix& oldMatrix = extendedRow->getSelectedEdges();
	selectedEdges = AdjacencyMatrix::introduce(oldMatrix, introducedIndex);

	const Row::VertexList& oldSelectedVertices = extendedRow->getSelectedVertices();
	assert(oldSelectedVertices.size() == oldMatrix.getNumRows());
	selectedVertices.resize(oldSelectedVertices.size() + 1);
	std::copy(oldSelectedVertices.begin(), oldSelectedVertices.begin() + introducedIndex, selectedVertices.begin());
	selectedVertices[introducedIndex] = false;
	std::copy(oldSelectedVertices.begin() + introducedIndex, oldSelectedVertices.end(), selectedVertices.begin() + introducedIndex + 1);
#ifndef NDEBUG
	{
		std::ostringstream msg;
		for(bool b : selectedVertices)
			msg << b;
		LOG("  resetting selected vertices to " << msg.str());
	}
#endif

	const AdjacencyMatrix& oldConnected = extendedRow->getConnectedViaSelectedEdges();
	assert(oldConnected.getNumRows() == oldMatrix.getNumRows());
	assert(oldConnected.isSymmetric());
	assert(connectedViaSelectedEdges.isSymmetric());
	connectedViaSelectedEdges = AdjacencyMatrix::introduce(oldConnected, introducedIndex);
	assert(connectedViaSelectedEdges.isSymmetric());
	LOG("  resetting connections to " << connectedViaSelectedEdges);

	hasForgottenComponent = extendedRow->getHasForgottenComponent();

	currentCost = extendedRow->getCurrentCost();
	intersectionCost = currentCost;
}

bool IntroductionSolver::nextGuess()
{
	// If we had forgotten a component, we may not select any edges anymore
	assert(hasForgottenComponent == getCurrentRowCombination().front()->getHasForgottenComponent());
	if(hasForgottenComponent) {
		assert(std::all_of(guessedEdges.begin(), guessedEdges.end(), [](bool s) { return !s; }));
		return false;
	}

	for(unsigned i = 0; i < guessedEdges.size(); ++i) {
		if(guessedEdges[i] == false) {
			guessedEdges[i] = true;
			return true;
		}
		guessedEdges[i] = false;
	}
	return false;
}

}} // namespace solver::steiner
