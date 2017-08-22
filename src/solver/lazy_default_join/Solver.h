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
#include "../../LazySolver.h"

namespace solver { namespace lazy_default_join {

class Solver : public ::LazySolver
{
public:
//	static unsigned joinSetups;
//	static unsigned joinCalls;
//	static unsigned discardedJoinResults;

	Solver(const Decomposition& decomposition, const Application& app, bool setLeavesToAccept, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full, bool binarySearch = true);

protected:
	virtual void startSolvingForCurrentRowCombination() override;
	virtual bool endOfRowCandidates() const override;
	virtual void nextRowCandidate() override;
	virtual void handleRowCandidate(long costBound) override;
	virtual bool resetRowIteratorsOnNewRow(Row newRow, const Decomposition& from) override;

private:
	bool binarySearch;
	ItemTreeNode::Type rowType; // ACCEPT or UNDEFINED
	bool currentRowCombinationExhausted;
};

}} // namespace solver::lazy_default_join
