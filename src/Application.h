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

#pragma once
//}}}
#include "options/OptionHandler.h"
#include "options/Choice.h"

class Decomposer;
class SolverFactory;
class Printer;

class Application
{
public:
	Application(const std::string& binaryName);

	int run(int argc, char** argv);

	// Print usage (but don't exit)
	void usage() const;

	const std::string& getInputString() const;
	options::OptionHandler& getOptionHandler();
	options::Choice& getDecomposerChoice();
	options::Choice& getSolverChoice();
	options::Choice& getPrinterChoice();

	const SolverFactory& getSolverFactory() const;
	Printer& getPrinter() const;

	void setDecomposer(Decomposer& decomposer);
	void setSolverFactory(SolverFactory& solverFactory);
	void setPrinter(Printer& printer);

	bool isCountingDisabled() const;
	bool isPruningDisabled() const;

	// Returns the depth until which the solution item tree should be materialized.
	// If the user did not specify a value, returns the greatest unsigned int.
	unsigned int getMaterializationDepth() const;

private:
	static const std::string MODULE_SECTION;

	std::string binaryName;
	std::string inputString;

	options::OptionHandler opts;
	options::Choice optDecomposer;
	options::Choice optSolver;
	options::Choice optPrinter;
	options::Option optNoCounting;
	options::Option optNoPruning;

	Decomposer* decomposer;
	SolverFactory* solverFactory;
	Printer* printer;

	unsigned int depth;
};
