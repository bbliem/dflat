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
#include "SolverFactory.h"
#include "Solver.h"
#include "../default_join/Solver.h"
#include "../lazy_default_join/Solver.h"
#include "../../Application.h"
#include "../../Decomposition.h"

namespace solver { namespace steiner {

const std::string SolverFactory::OPTION_SECTION = "Steiner Tree solver";

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "steiner", "Steiner Tree solver", newDefault)
	, optDefaultJoin    ("default-join",     "Use built-in implementation for join nodes")
	, optLazy           ("lazy",             "Use lazy evaluation to find one solution")
	, optNoBinarySearch ("no-binary-search", "Disable binary search in lazy default join")
	, optBbLevel        ("bb", "s",          "Use branch and bound stategy <s> for lazy solving")
{
	optDefaultJoin.addCondition(selected);
	app.getOptionHandler().addOption(optDefaultJoin, OPTION_SECTION);

	optLazy.addCondition(selected);
	app.getOptionHandler().addOption(optLazy, OPTION_SECTION);

	optNoBinarySearch.addCondition(selected);
	optNoBinarySearch.addCondition(condLazy);
	optNoBinarySearch.addCondition(condDefaultJoin);
	app.getOptionHandler().addOption(optNoBinarySearch, OPTION_SECTION);

	optBbLevel.addCondition(selected);
	optBbLevel.addCondition(condLazy);
	optBbLevel.addCondition(condOptimization);
	optBbLevel.addChoice("none", "No branch and bound");
	optBbLevel.addChoice("basic", "Prevent rows not cheaper than current provisional solution");
	optBbLevel.addChoice("full", "Improve bounds using solutions for forgotten subgraphs", true);
	app.getOptionHandler().addOption(optBbLevel, OPTION_SECTION);
}

std::unique_ptr<::Solver> SolverFactory::newSolver(const Decomposition& decomposition) const
{
	if(optLazy.isUsed()) {
		assert(!optNoBinarySearch.isUsed() || optDefaultJoin.isUsed());
		LazySolver::BranchAndBoundLevel bbLevel;
		if(optBbLevel.getValue() == "none")
			bbLevel = LazySolver::BranchAndBoundLevel::none;
		else if(optBbLevel.getValue() == "basic")
			bbLevel = LazySolver::BranchAndBoundLevel::basic;
		else {
			assert(optBbLevel.getValue() == "full");
			bbLevel = LazySolver::BranchAndBoundLevel::full;
		}
		if(optDefaultJoin.isUsed() && decomposition.isJoinNode())
			return std::unique_ptr<::Solver>(new lazy_default_join::Solver(decomposition, app, bbLevel, !optNoBinarySearch.isUsed()));
		else
			return std::unique_ptr<::Solver>(new steiner::Solver(decomposition, app, bbLevel));
	}
	else {
		if(optDefaultJoin.isUsed() && decomposition.isJoinNode())
			return std::unique_ptr<::Solver>(new default_join::Solver(decomposition, app));
		else
			//return std::unique_ptr<::Solver>(new clasp::Solver(decomposition, app, optEncodingFiles.getValues(), optTables.isUsed(), optCardinalityCost.isUsed(), optPrintStatistics.isUsed()));
			return {}; // TODO
	}
}

void SolverFactory::notify()
{
	::SolverFactory::notify();

	if(optLazy.isUsed())
		condLazy.setSatisfied();

	if(optDefaultJoin.isUsed())
		condDefaultJoin.setSatisfied();

	if(!app.isOptimizationDisabled())
		condOptimization.setSatisfied();
}

}} // namespace solver::steiner
