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

	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool reground = false, BranchAndBoundLevel bbLevel = BranchAndBoundLevel::full, bool cardinalityCost = false);

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
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CurrentCounterAtomArguments> CurrentCounterAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CounterAtomArguments> CounterAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CurrentCounterIncAtomArguments> CurrentCounterIncAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CounterIncAtomArguments> CounterIncAtomInfo;
	typedef asp_utils::ClaspAtomInfo<GringoOutputProcessor::CounterRemAtomArguments> CounterRemAtomInfo;

	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator () (std::pair<T1,T2> const& p) const {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return h1^h2;
		}
	};
	using Custom_map = std::unordered_map<std::pair<std::string,long>, size_t, pair_hash>;

	std::vector<ItemAtomInfo>										itemAtomInfos;
	std::vector<AuxItemAtomInfo>									auxItemAtomInfos;
	std::vector<CurrentCostAtomInfo>								currentCostAtomInfos;
	std::vector<CostAtomInfo>										costAtomInfos;
	std::vector<CurrentCounterAtomInfo>								currentCounterAtomInfos;
	std::vector<CounterAtomInfo>									counterAtomInfos;
	std::vector<CurrentCounterIncAtomInfo>							currentCounterIncAtomInfos;
	std::vector<CounterIncAtomInfo>									counterIncAtomInfos;
	std::map<std::string,Clasp::Literal>                            counterRemAtomInfos;
	std::map<std::string,std::vector<CounterIncAtomInfo>>           allCounterIncAtomInfos;
	std::map<std::string,std::vector<CurrentCounterIncAtomInfo>>    allCurrentCounterIncAtomInfos;
	std::map<std::string,std::vector<CounterAtomInfo>>              allCounterAtomInfos;
	std::map<std::string,std::vector<CurrentCounterAtomInfo>>       allCurrentCounterAtomInfos;

	bool reground;
	bool cardinalityCost;
	std::vector<std::string> encodingFiles;
	std::vector<Clasp::Literal> literals;
	std::unordered_map<String, size_t> itemsToLitIndices;
	std::unordered_map<String, size_t> auxItemsToLitIndices;
	Custom_map countersToLitIndices;
	std::unordered_map<long, size_t> costsToLitIndices;

	Clasp::ClaspFacade clasp;
	Clasp::ClaspConfig config;
	std::unique_ptr<SolveIter> asyncResult;
};

}} // namespace solver::lazy_clasp
