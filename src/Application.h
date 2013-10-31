/*
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

#include "options/OptionHandler.h"
#include "options/Choice.h"

class Decomposer;
class SolverFactory;
class Debugger;

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
	options::Choice& getDebuggerChoice();

	const SolverFactory& getSolverFactory() const;
	const Debugger& getDebugger() const;

	void setDecomposer(Decomposer& decomposer);
	void setSolverFactory(SolverFactory& solverFactory);
	void setDebugger(Debugger& debugger);

	bool isPruningDisabled() const;

private:
	static const std::string MODULE_SECTION;

	std::string binaryName;
	std::string inputString;

	options::OptionHandler opts;
	options::Choice optDecomposer;
	options::Choice optSolver;
	options::Choice optDebugger;
	options::Option optNoPruning;

	Decomposer* decomposer;
	SolverFactory* solverFactory;
	Debugger* debugger;
};
