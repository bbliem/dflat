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
#include <iostream>

#include "Debug.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace printer {

const std::string Debug::OPTION_SECTION = "Human-readable debugging output";

Debug::Debug(Application& app, bool newDefault)
	: Printer(app, "debug", "Human-readable debugging output", newDefault)
	, optPrintSolverEvents("print-solver-events", "Print events that occurred during solving")
	, optPrintSolverInvocationInput("print-solver-input", "Print solver invocation input")
{
	optPrintSolverEvents.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverEvents, OPTION_SECTION);

	optPrintSolverInvocationInput.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverInvocationInput, OPTION_SECTION);
}

void Debug::decomposerResult(const Decomposition& result)
{
	std::cout << "Decomposition (width " << result.getWidth() << "):" << std::endl << result << std::endl;
}

void Debug::solverInvocationInput(const Decomposition& decompositionNode, const std::string& input)
{
	if(optPrintSolverInvocationInput.isUsed())
		std::cout << "Input for solver at decomposition node " << decompositionNode.getNode().getGlobalId() << ':' << std::endl << input << std::endl;
}

void Debug::solverInvocationResult(const Decomposition& decompositionNode, const Table* result)
{
	std::cout << std::endl << "Resulting table (";
	if(result)
		std::cout << result->getRows().size();
	else
		std::cout << '0';
	std::cout << " rows) at decomposition node " << decompositionNode.getNode().getGlobalId() << ':' << std::endl;
	if(result)
		std::cout << *result;
	else
		std::cout << "(empty)";
	std::cout << std::endl;
}

bool Debug::listensForSolverEvents() const
{
	return optPrintSolverEvents.isUsed();
}

void Debug::solverEvent(const std::string& msg)
{
	std::cout << msg << std::endl;
}

} // namespace printer
