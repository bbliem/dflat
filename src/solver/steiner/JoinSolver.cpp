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
#include "JoinSolver.h"

namespace solver { namespace steiner {

JoinSolver::JoinSolver(const Decomposition& decomposition, const Application& app, BranchAndBoundLevel bbLevel, bool binarySearch)
	: Solver(decomposition, app, bbLevel)
	, binarySearch(binarySearch)
{
	assert(decomposition.getChildren().size() > 1);
}

void JoinSolver::nextRowCandidate()
{
	Solver::nextRowCandidate();
	end = true;
}

void JoinSolver::startSolvingForCurrentRowCombination()
{
	Solver::startSolvingForCurrentRowCombination();

	const auto& extended = getCurrentRowCombination();
	assert(extended.empty() == false);
	const auto& firstExtendedRow = **extended.begin();

	unsigned childrenWithForgottenComponent = 0;
	for(const auto& ep : extended)
		childrenWithForgottenComponent += ep->getHasForgottenComponent();
	if(childrenWithForgottenComponent > 1) {
		end = true;
		return;
	}
	hasForgottenComponent = childrenWithForgottenComponent > 0;

	selectedVertices = firstExtendedRow.getSelectedVertices();
	if(hasForgottenComponent) {
		bool selectedVertexExists = false;
		for(bool b : selectedVertices)
			selectedVertexExists = selectedVertexExists || b;
		if(selectedVertexExists) {
			end = true;
			return;
		}
	}

	selectedEdges = firstExtendedRow.getSelectedEdges();
	// If we used binary search, we are guaranteed that at least the selected edges match.
	assert(!binarySearch || std::all_of(extended.begin(), extended.end(), [this](const Row::ExtensionPointer& row) {
			return row->getSelectedEdges() == this->selectedEdges;
	}));

	connectedViaSelectedEdges = firstExtendedRow.getConnectedViaSelectedEdges();
	const unsigned size = selectedVertices.size();
	assert(connectedViaSelectedEdges.getNumRows() == size);
	currentCost = firstExtendedRow.getCurrentCost();
	intersectionCost = currentCost; // TODO check if that's correct

	for(auto it = extended.begin() + 1; it != extended.end(); ++it) {
		const auto& extendedSelectedEdges = (*it)->getSelectedEdges();
		const auto& extendedSelectedVertices = (*it)->getSelectedVertices();
		const auto& extendedConnected = (*it)->getConnectedViaSelectedEdges();
		assert(extendedSelectedEdges.getNumRows() == size);
		assert(extendedSelectedVertices.size() == size);
		assert(extendedConnected.getNumRows() == size);

		if(!binarySearch) { // If we use binary search, the selected edges are guaranteed to be the same
			if(!(selectedEdges == extendedSelectedEdges)) {
				end = true;
				return;
			}
		}
		assert((*it)->getCurrentCost() == currentCost);

		bool selectedVertexExists = false;
		for(unsigned i = 0; i < size; ++i) {
			selectedVertexExists = selectedVertexExists || extendedSelectedVertices[i];
			selectedVertices[i] = selectedVertices[i] || extendedSelectedVertices[i];
		}
		if(hasForgottenComponent && selectedVertexExists) {
			end = true;
			return;
		}

		connectedViaSelectedEdges.bitwiseOr(extendedConnected);
	}
	connectedViaSelectedEdges.makeTransitive();
}

bool JoinSolver::resetRowIteratorsOnNewRow(Rows::const_iterator newRow, const Decomposition& from)
{
	if(!binarySearch)
		return LazySolver::resetRowIteratorsOnNewRow(newRow, from);

	rowIterators.clear();
	for(const auto& child : decomposition.getChildren()) {
		if(child.get() == &from) {
			Rows::const_iterator end = newRow;
			++end;
			rowIterators.push_back({newRow, newRow, end});
		} else {
			const auto& rows = static_cast<LazySolver&>(child->getSolver()).getTable()->getRows();
			assert(rows.begin() != rows.end());
			const auto range = std::equal_range(rows.begin(), rows.end(), *newRow,
					[](const RowPtr& a, const RowPtr& b) {
					// FIXME Incomplete?
					return a->getSelectedEdges() < b->getSelectedEdges();
					});
			if(range.first == range.second)
				return false;
			rowIterators.push_back({range.first, range.first, range.second});
		}
	}

	return true;
}

}} // namespace solver::steiner
