/*{{{
Copyright 2012-2015, Bernhard Bliem
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

#include "ClaspCallback.h"
#include "../../Decomposition.h"
#include "../../LazySolver.h"
#include "SolverIter.h"

namespace solver { namespace lazy_clasp {

class Solver : public ::LazySolver
{
public:
	Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles);

protected:
	virtual const ItemTreePtr& getItemTree() const override;
	virtual void setItemTree(ItemTreePtr&& itemTree) override;
	virtual ItemTree::Children::const_iterator getNewestRow() const override;
	virtual ItemTreePtr finalize() override;

	virtual void startSolvingForCurrentRowCombination() override;
	virtual bool endOfRowCandidates() override;
	virtual void nextRowCandidate() override;
	virtual void handleRowCandidate() override;

private:
	std::vector<std::string> encodingFiles;
	std::vector<Clasp::Var> variables;
	std::unordered_map<String, size_t> itemsToVarIndices;

	std::unique_ptr<ClaspCallback> claspCallback;
	std::unique_ptr<Gringo::Output::LparseOutputter> lpOut;

	Clasp::ClaspFacade clasp;
	Clasp::ClaspConfig config;
	std::unique_ptr<SolveIter> asyncResult;
};

}} // namespace solver::lazy_clasp
