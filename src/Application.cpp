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

#include <cstdlib>
#include <iostream>
#include <sstream>

#include "Application.h"

#include "options/Choice.h"
#include "options/MultiValueOption.h"
#include "options/SingleValueOption.h"
#include "options/Condition.h"
#include "options/OptionHandler.h"
#include "options/HelpObserver.h"

#include "Decomposer.h"
#include "decomposer/Dummy.h"
#include "decomposer/TdDecomposer.h"

#include "Solver.h"
#include "solver/Dummy.h"

#include "Traverser.h"
#include "traverser/Dummy.h"

const std::string Application::MODULE_SECTION = "Module selection";

Application::Application(const std::string& binaryName)
: binaryName(binaryName)
{
}

void Application::run(int argc, char** argv)
{
	// Set up general options
	options::Option optHelp("h", "Print usage information and exit");
	opts.addOption(optHelp);
	// Register an observer for printing the usage when -h is supplied.
	// Note that we use an observer for this instead of checking manually (right after opts.parse()) if optHelp was used, because other observers might perform actions which are undesired if -h has been passed. This is why helpObserver is the first observer we register.
	options::HelpObserver helpObserver(*this, optHelp);
	opts.registerObserver(helpObserver);

	options::MultiValueOption optEdge("e", "edge", "Predicate <edge> declares (hyper)edges");
	opts.addOption(optEdge);

	options::SingleValueOption optSeed("seed", "n", "Initialize random number generator with seed <n>");
	opts.addOption(optSeed);

	// Set up module selection options
	options::Choice optDecomposer("d", "decomposer", "Use decomposition method <decomposer>");
	opts.addOption(optDecomposer, MODULE_SECTION);
	decomposer::Dummy dummyDecomposer(optDecomposer);
	decomposer::TdDecomposer tdDecomposer(opts, optDecomposer, true);

	options::Choice optSolver("s", "solver", "Use <solver> to compute partial solutions");
	solver::Dummy dummySolver(optSolver, true);
	opts.addOption(optSolver, MODULE_SECTION);

	options::Choice optTraverser("t", "traverser", "Use <traverser> to apply the solver on the decomposition");
	traverser::Dummy dummyTraverser(optTraverser, true);
	opts.addOption(optTraverser, MODULE_SECTION);

	//		options::Choice optFinalizer("f", "finalizer", "Use <finalizer> to materialize complete solutions");
	//		optFinalizer.addChoice("enumeration", "Enumerate all solutions", true);
	//		optFinalizer.addChoice("search", "Enumerate one solution");
	//		optFinalizer.addChoice("counting", "Print number of solutions");
	//		optFinalizer.addChoice("decision", "Report whether there is a solution");
	//		optFinalizer.addChoice("none", "Do nothing");
	//		opts.addOption(optFinalizer);


	// Parse command line
	opts.parse(argc, argv);

	// Set random seed
	time_t seed = time(0);
	if(optSeed.isUsed()) {
		char* endptr;
		seed = strtol(optSeed.getValue().c_str(), &endptr, 0);
		if(*endptr) {
			std::cerr << "Invalid seed" << std::endl;
			usage(2);
		}
	}
	srand(seed);

	// Store all of stdin in a string
	std::ostringstream inputStringStream;
	inputStringStream << std::cin.rdbuf();
	std::string inputString = inputStringStream.str();

	// Parse instance
//	parser::Driver driver(*this, inputString);
//	driver.parse();
//	Problem problem(inputString, hyperedgePredicateNames);
//
//	sharp::ExtendedHypertree* decomposition = problem.calculateHypertreeDecomposition();
}

void Application::usage(int exitCode) const
{
	std::cerr << "Usage: " << binaryName << " [options] < instance" << std::endl;
	opts.printHelp();
	std::exit(exitCode);
}
