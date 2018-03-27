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

#include "Application.h"
#include "Decomposition.h"
#include "Printer.h"

Printer::NodeStackElement::NodeStackElement(Printer& printer, const Decomposition& decompositionNode)
	: printer(printer)
{
	printer.enterNode(decompositionNode);
}

Printer::NodeStackElement::~NodeStackElement()
{
	printer.leaveNode();
}

Printer::Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault)
	: Module(app, app.getPrinterChoice(), optionName, optionDescription, newDefault)
{
}

Printer::~Printer()
{
}

void Printer::decomposerResult(const Decomposition& result)
{
	if(app.printDecomposition())
		std::cout << "Decomposition (width " << result.getWidth() << "):" << std::endl << result << std::endl;
}

Printer::NodeStackElement Printer::visitNode(const Decomposition& decompositionNode)
{
	return NodeStackElement(*this, decompositionNode);
}

void Printer::solverInvocationInput(const Decomposition& decompositionNode, const std::string& input)
{
}

void Printer::solverInvocationResult(const Decomposition& decompositionNode, const Table* result)
{
}

bool Printer::listensForSolverEvents() const
{
	return false;
}

void Printer::solverEvent(const std::string& msg)
{
}

void Printer::provisionalSolution(const Row& solution)
{
	if(app.printProvisionalSolutions()) {
		std::cout << "Provisional solution:" << std::endl;
		for(const auto& item : solution.firstExtension())
			std::cout << item << ' ';
		std::cout << std::endl << "Cost: " << solution.getCost() << std::endl;
	}
}

void Printer::result(const TablePtr& rootTable)
{
	std::cout << "Solutions:" << std::endl;
	if(rootTable)
		rootTable->printExtensions(std::cout);
	else
		std::cout << "[0]" << std::endl;
}

void Printer::select()
{
	Module::select();
	app.setPrinter(*this);
}

void Printer::enterNode(const Decomposition& decompositionNode)
{
}

void Printer::leaveNode()
{
}
