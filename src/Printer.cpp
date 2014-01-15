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

#include "Application.h"
#include "Printer.h"

Printer::Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault)
	: Module(app, app.getPrinterChoice(), optionName, optionDescription, newDefault)
{
}

Printer::~Printer()
{
}

void Printer::decomposerResult(const Decomposition& result)
{
}

void Printer::solverInvocationInput(const DecompositionNode& decompositionNode, const std::string& input)
{
}

void Printer::solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result)
{
}

bool Printer::listensForSolverEvents() const
{
	return false;
}

void Printer::solverEvent(const std::string& msg)
{
}

void Printer::result(const ItemTreePtr& rootItemTree)
{
	std::cout << "Solutions:" << std::endl;
	if(rootItemTree)
		rootItemTree->printExtensions(std::cout, app.getMaterializationDepth(), !app.isCountingDisabled());
	else
		std::cout << "[0]" << std::endl;
}

void Printer::select()
{
	Module::select();
	app.setPrinter(*this);
}
