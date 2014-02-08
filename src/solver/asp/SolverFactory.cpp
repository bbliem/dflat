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

#ifdef HAVE_WORDEXP_H
#include <string>
#include <fstream>
#include <wordexp.h>
#endif

namespace solver { namespace asp {

const std::string SolverFactory::OPTION_SECTION = "ASP solver";

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "asp", "Answer Set Programming", newDefault)
	, optEncodingFiles  ("p", "program",     "Use <program> as an ASP encoding for solving")
	, optDefaultJoin    ("default-join",     "Use built-in implementation for join nodes")
	, optLazy           ("lazy",             "Use lazy evaluation")
	, optTables         ("tables",           "Use table mode (for item trees of height at most 1)")
#ifdef HAVE_WORDEXP_H
	, optIgnoreModelines("ignore-modelines", "Do not scan the encoding files for modelines")
#endif
{
	optEncodingFiles.addCondition(selected);
	app.getOptionHandler().addOption(optEncodingFiles, OPTION_SECTION);

	optDefaultJoin.addCondition(selected);
	app.getOptionHandler().addOption(optDefaultJoin, OPTION_SECTION);

	optLazy.addCondition(selected);
	app.getOptionHandler().addOption(optLazy, OPTION_SECTION);

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

#ifdef HAVE_WORDEXP_H
void SolverFactory::notify()
{
	::SolverFactory::notify();

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
				std::getline(file, firstLine);
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
}
#endif

}} // namespace solver::asp
