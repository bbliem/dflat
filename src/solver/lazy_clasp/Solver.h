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
#include <clasp/clasp_facade.h>

#include "../../asp_utils.h"
#include "../../Decomposition.h"
#include "../../LazySolver.h"
#include "GringoOutputProcessor.h"
#include "SolveIter.h"

namespace solver { namespace lazy_clasp {

class Solver : public ::LazySolver
{
public:
//	static unsigned solverSetups;
//	static unsigned solveCalls;
//	static unsigned models;
//	static unsigned discardedModels;

	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool reground = false, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full);

protected:
	virtual void startSolvingForCurrentRowCombination() override;
	virtual bool endOfRowCandidates() const override;
	virtual void nextRowCandidate() override;
	virtual void handleRowCandidate(long costBound) override;

private:
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::ItemAtomArguments> ItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::AuxItemAtomArguments> AuxItemAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CurrentCostAtomArguments> CurrentCostAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CostAtomArguments> CostAtomInfo;

	std::vector<ItemAtomInfo>        itemAtomInfos;
	std::vector<AuxItemAtomInfo>     auxItemAtomInfos;
//	std::vector<CurrentCostAtomInfo> currentCostAtomInfos;
//	std::vector<CostAtomInfo>        costAtomInfos;

	bool reground;
	std::vector<std::string> encodingFiles;
	std::vector<Clasp::Literal> literals;
	std::unordered_map<String, size_t> itemsToLitIndices;
	std::unordered_map<String, size_t> auxItemsToLitIndices;

	Clasp::ClaspFacade clasp;
	Clasp::ClaspConfig config;
	std::unique_ptr<SolveIter> asyncResult;
};

}} // namespace solver::lazy_clasp
