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

#include "DebugHumanReadable.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace {

	size_t countNodes(const ItemTree& tree)
	{
		size_t nodes = 1;

		for(const auto& child : tree.getChildren())
			nodes += countNodes(*child);

		return nodes;
	}

} // anonymous namespace

namespace printer {

const std::string DebugHumanReadable::OPTION_SECTION = "Human-readable debugging output";

DebugHumanReadable::DebugHumanReadable(Application& app, bool newDefault)
	: Printer(app, "human", "Human-readable debugging output", newDefault)
	, optPrintSolverEvents("print-solver-events", "Print events that occurred during solving")
	, optPrintSolverInvocationInput("print-solver-input", "Print solver invocation input")
	, optPrintUncompressedItemTrees("print-uncompressed", "Print item trees before compression")
{
	optPrintSolverEvents.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverEvents, OPTION_SECTION);

	optPrintSolverInvocationInput.addCondition(selected);
	app.getOptionHandler().addOption(optPrintSolverInvocationInput, OPTION_SECTION);

	optPrintUncompressedItemTrees.addCondition(selected);
	app.getOptionHandler().addOption(optPrintUncompressedItemTrees, OPTION_SECTION);
}

void DebugHumanReadable::decomposerResult(const Decomposition& result)
{
	std::cout << "Decomposition (width " << result.getWidth() << "):" << std::endl << result << std::endl;
}

void DebugHumanReadable::solverInvocationInput(const Decomposition& decompositionNode, const std::string& input)
{
	if(optPrintSolverInvocationInput.isUsed())
		std::cout << "Input for solver at decomposition node " << decompositionNode.getNode().getGlobalId() << ':' << std::endl << input << std::endl;
}

void DebugHumanReadable::solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result)
{
	std::cout << std::endl << "Resulting item tree (";
	if(result)
		std::cout << countNodes(*result);
	else
		std::cout << '0';
	std::cout << " nodes) at decomposition node " << decompositionNode.getNode().getGlobalId() << ':' << std::endl;
	if(result)
		std::cout << *result;
	else
		std::cout << "(empty)";
	std::cout << std::endl;
}

void DebugHumanReadable::uncompressedSolverInvocationResult(const Decomposition& decompositionNode, const UncompressedItemTree* result)
{
	if(optPrintUncompressedItemTrees.isUsed()) {
		std::cout << std::endl << "Uncompressed item tree at decomposition node " << decompositionNode.getNode().getGlobalId();
		if(result)
			std::cout << ':' << std::endl << *result;
		else
			std::cout << " is empty.";
		std::cout << std::endl;
	}
}

bool DebugHumanReadable::listensForSolverEvents() const
{
	return optPrintSolverEvents.isUsed();
}

void DebugHumanReadable::solverEvent(const std::string& msg)
{
	std::cout << msg << std::endl;
}

} // namespace printer
