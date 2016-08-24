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
#include "../lazy_clasp/Solver.h"
#include "../lazy_default_join/Solver.h"
#include "../../Application.h"
#include "../../Decomposition.h"

#ifdef HAVE_WORDEXP_H
#include <string>
#include <fstream>
#include <wordexp.h>
#endif

namespace solver { namespace clasp {

const std::string SolverFactory::OPTION_SECTION = "Clasp solver";

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "clasp", "Answer Set Programming solver clasp", newDefault)
	, optEncodingFiles  ("p", "program",     "Use <program> as an ASP encoding for solving")
	, optCardinalityCost("cardinality-cost", "Use item set cardinality as costs")
	, optPrintStatistics("stats",            "Print clasp statistics")
	, optDefaultJoin    ("default-join",     "Use built-in implementation for join nodes")
	, optLazy           ("lazy",             "Use lazy evaluation to find one solution")
	, optNoBinarySearch ("no-binary-search", "Disable binary search in lazy default join")
	, optBbLevel        ("bb", "s",          "Use branch and bound stategy <s> for lazy solving")
	, optReground       ("reground",         "Reground instead of external atoms in lazy solving")
	, optTables         ("tables",           "Use table mode (for item trees of height at most 1)")
#ifdef HAVE_WORDEXP_H
	, optIgnoreModelines("ignore-modelines", "Do not scan the encoding files for modelines")
#endif
{
	optEncodingFiles.addCondition(selected);
	app.getOptionHandler().addOption(optEncodingFiles, OPTION_SECTION);

	optCardinalityCost.addCondition(selected);
	optCardinalityCost.addCondition(condTables);
	app.getOptionHandler().addOption(optCardinalityCost, OPTION_SECTION);

	optPrintStatistics.addCondition(selected);
	app.getOptionHandler().addOption(optPrintStatistics, OPTION_SECTION);

	optDefaultJoin.addCondition(selected);
	app.getOptionHandler().addOption(optDefaultJoin, OPTION_SECTION);

	optLazy.addCondition(selected);
	optLazy.addCondition(condTables); // TODO Lazy solving should not require table mode?
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

	optReground.addCondition(selected);
	optReground.addCondition(condLazy);
	app.getOptionHandler().addOption(optReground, OPTION_SECTION);

	optTables.addCondition(selected);
	app.getOptionHandler().addOption(optTables, OPTION_SECTION);

#ifdef HAVE_WORDEXP_H
	optIgnoreModelines.addCondition(selected);
	app.getOptionHandler().addOption(optIgnoreModelines, OPTION_SECTION);
#endif
}

std::unique_ptr<::Solver> SolverFactory::newSolver(const Decomposition& decomposition) const
{
	if(optLazy.isUsed()) {
		assert(optTables.isUsed() && condTables.isSatisfied());
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
			return std::unique_ptr<::Solver>(new lazy_default_join::Solver(decomposition, app, decomposition.isRoot(), bbLevel, !optNoBinarySearch.isUsed()));
		else
			return std::unique_ptr<::Solver>(new lazy_clasp::Solver(decomposition, app, optEncodingFiles.getValues(), optReground.isUsed(), bbLevel));
	}
	else {
		if(optDefaultJoin.isUsed() && decomposition.isJoinNode())
			return std::unique_ptr<::Solver>(new default_join::Solver(decomposition, app, optTables.isUsed() && decomposition.isRoot()));
		else
			return std::unique_ptr<::Solver>(new clasp::Solver(decomposition, app, optEncodingFiles.getValues(), optTables.isUsed(), optCardinalityCost.isUsed(), optPrintStatistics.isUsed()));
	}
}

void SolverFactory::select()
{
	::SolverFactory::select();
	if(!optEncodingFiles.isUsed())
		throw std::runtime_error("Clasp solver requires at least one program to be specified");
}

void SolverFactory::notify()
{
	::SolverFactory::notify();

	if(optLazy.isUsed())
		condLazy.setSatisfied();

	if(optTables.isUsed())
		condTables.setSatisfied();

	if(optDefaultJoin.isUsed())
		condDefaultJoin.setSatisfied();

	if(!app.isOptimizationDisabled())
		condOptimization.setSatisfied();

#ifdef HAVE_WORDEXP_H
	if(!optIgnoreModelines.isUsed()) {
		// Scan for modelines in encoding files
		for(const std::string& filename : optEncodingFiles.getValues()) {
			// Avoid infinite recursions
			if(std::find(modelineStack.begin(), modelineStack.end(), filename) != modelineStack.end())
				continue;
			modelineStack.push_back(filename);
			std::string firstLine;
			{
				std::ifstream file(filename);
				//std::getline(file, firstLine); // doesn't handle foreign line endings
				char c;
				while(file.get(c) && c != '\n' && c != '\r')
					firstLine.push_back(c);
			}
			static const std::string modelinePrefix = "%dflat: ";
			if(firstLine.substr(0, modelinePrefix.size()) == modelinePrefix) {
				wordexp_t p;
				try {
					if(wordexp(firstLine.substr(modelinePrefix.size()).c_str(), &p, 0) != 0)
						throw std::runtime_error("Error parsing modeline");
					app.getOptionHandler().parse(p.we_wordc, p.we_wordv);
				}
				catch(...) {
					wordfree(&p);
					throw;
				}
				wordfree(&p);
			}
			modelineStack.pop_back();
		}
	}
#endif
}

}} // namespace solver::clasp
