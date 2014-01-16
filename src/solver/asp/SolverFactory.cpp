/*{{{
Copyright 2012-2013, Bernhard Bliem
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
#include "../lazy_asp/Solver.h"
#include "../../Application.h"
#include "../../Decomposition.h"

namespace solver { namespace asp {

const std::string SolverFactory::OPTION_SECTION = "ASP solver";

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "asp", "Answer Set Programming", newDefault)
	, optEncodingFiles("p", "program", "Use <program> as an ASP encoding for solving")
	, optDefaultJoin  ("default-join", "Use built-in implementation for join nodes")
	, optLazy         ("lazy",         "Use lazy evaluation")
	, optTables       ("tables",       "Use table mode (for item trees of height at most 1)")
{
	optEncodingFiles.addCondition(selected);
	app.getOptionHandler().addOption(optEncodingFiles, OPTION_SECTION);

	optDefaultJoin.addCondition(selected);
	app.getOptionHandler().addOption(optDefaultJoin, OPTION_SECTION);

	optLazy.addCondition(selected);
	app.getOptionHandler().addOption(optLazy, OPTION_SECTION);

	optTables.addCondition(selected);
	app.getOptionHandler().addOption(optTables, OPTION_SECTION);
}

std::unique_ptr<::Solver> SolverFactory::newSolver(const Decomposition& decomposition) const
{
	if(optLazy.isUsed()) {
		// FIXME this should not make --default-join ineffective, and it should not require table mode
		if(optDefaultJoin.isUsed() || !optTables.isUsed())
			throw std::runtime_error("Lazy evaluation currently requires table mode and not using the default join");
		return std::unique_ptr<::Solver>(new lazy_asp::Solver(decomposition, app, optEncodingFiles.getValues()));
	}
	else {
		if(optDefaultJoin.isUsed() && decomposition.isJoinNode())
			return std::unique_ptr<::Solver>(new default_join::Solver(decomposition, app));
		else
			return std::unique_ptr<::Solver>(new asp::Solver(decomposition, app, optEncodingFiles.getValues(), optTables.isUsed()));
	}
}

void SolverFactory::select()
{
	::SolverFactory::select();
	if(!optEncodingFiles.isUsed())
		throw std::runtime_error("ASP solver requires at least one program to be specified");
}


}} // namespace solver::asp
