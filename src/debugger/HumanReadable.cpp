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
#include <iostream>

#include "HumanReadable.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace debugger {

const std::string HumanReadable::OPTION_SECTION = "Human-readable debugging output";

HumanReadable::HumanReadable(Application& app, bool newDefault)
	: Debugger(app, "human", "Human-readable debugging output", newDefault)
	, optPrintSolverEvents("print-solver-events", "Print events that occurred during solving")
	, optPrintSolverInvocationInput("print-solver-input", "Print solver invocation input")
{
	optPrintSolverEvents.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverEvents, OPTION_SECTION);

	optPrintSolverInvocationInput.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverInvocationInput, OPTION_SECTION);
}

void HumanReadable::decomposerResult(const Decomposition& result) const
{
	std::cout << "Decomposition:" << std::endl << result << std::endl;
}

void HumanReadable::solverInvocationInput(const DecompositionNode& decompositionNode, const std::string& input) const
{
	if(optPrintSolverInvocationInput.isUsed()) {
		std::cout << "Input for solver at decomposition node " << decompositionNode.getGlobalId() << ':' << std::endl << input << std::endl;
	}
}

void HumanReadable::solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result) const
{
	std::cout << std::endl << "Resulting item tree at decomposition node " << decompositionNode.getGlobalId();
	if(result)
		std::cout << ':' << std::endl << *result;
	else
		std::cout << " is empty.";
	std::cout << std::endl;
}

bool HumanReadable::listensForSolverEvents() const
{
	return optPrintSolverEvents.isUsed();
}

void HumanReadable::solverEvent(const std::string& msg) const
{
	std::cout << msg << std::endl;
}

} // namespace debugger
