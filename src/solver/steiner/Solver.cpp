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

namespace solver { namespace steiner {

Solver::Solver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel)
	: ::LazySolver(decomposition, app, bbLevel)
{
}

void Solver::startSolvingForCurrentRowCombination()
{
	// TODO
}

bool Solver::endOfRowCandidates() const
{
	// TODO
	return true;
}

void Solver::nextRowCandidate()
{
	// TODO
}

void Solver::handleRowCandidate(long costBound)
{
	assert(table);
	Row::Items items;
	Row::Items auxItems;

	// TODO

	std::shared_ptr<Row> row(new Row(std::move(items), std::move(auxItems), {getCurrentRowCombination()}));

	if(!app.isOptimizationDisabled()) {
		long cost = 0;
		long currentCost = 0;

		// TODO

		if(cost >= costBound) {
			newestRow = table->getRows().end();
			return;
		}

		row->setCost(cost);
		row->setCurrentCost(currentCost);
	}

	newestRow = table->add(std::move(row));

	//if(newChild != itemTree->getChildren().end())
	//	newestRow = newChild;
}

}} // namespace solver::steiner
