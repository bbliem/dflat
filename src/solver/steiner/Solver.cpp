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

#include "Solver.h"
#include "../../Application.h"
#include "../../Printer.h"
#include "../../Table.h"
#include "../../Decomposition.h"
#include "../../Application.h"

// TODO remove
#ifndef NDEBUG
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

Solver::Solver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel)
	: ::LazySolver(decomposition, app, bbLevel)
{
}

void Solver::startSolvingForCurrentRowCombination()
{
	LOG("startSolvingForCurrentRowCombination");
	end = false;
	hasForgottenComponent = false;
	currentCost = 0;
	intersectionCost = 0;
}

bool Solver::endOfRowCandidates() const
{
	LOG("endOfRowCandidates returning " << end);
	return end;
}

void Solver::nextRowCandidate()
{
	LOG("nextRowCandidate");
}

void Solver::handleRowCandidate(unsigned long costBound)
{
	LOG("handleRowCandidate");
	assert(table);
	assert(!end);

	LOG("  Selected edges: " << selectedEdges);
	LOG("  Connections: " << connectedViaSelectedEdges);
	std::shared_ptr<Row> row(new Row(std::move(selectedEdges), std::move(selectedVertices), std::move(connectedViaSelectedEdges), hasForgottenComponent, {getCurrentRowCombination()}));

	if(!app.isOptimizationDisabled()) {
		unsigned long cost = currentCost;
		for(const auto& childRow : row->getExtensionPointers().front())
			cost += childRow->getCost() - intersectionCost;

		if(cost >= costBound) {
			LOG("  Cost " << cost << " not below bound " << costBound);
			newestRow = table->getRows().end();
			return;
		}

		LOG("  Setting cost to " << cost << "; current cost to " << currentCost);
		row->setCost(cost);
		row->setCurrentCost(currentCost);
	}

	LOG("  Adding row " << *row);
	newestRow = table->add(std::move(row));
#ifndef NDEBUG
	if(newestRow == table->getRows().end()) {
		LOG("  Failed adding row");
	} else {
		LOG("  Successfully added row");
	}
#endif
}

}} // namespace solver::steiner
