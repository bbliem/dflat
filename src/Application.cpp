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
#include <fstream>
#include <sstream>
#include <cassert>

#include "Application.h"
#include "Decomposition.h"
#include "version.h"

#include "options/MultiValueOption.h"
#include "options/SingleValueOption.h"
#include "options/OptionHandler.h"
#include "options/HelpObserver.h"
#include "options/VersionObserver.h"

#include "decomposer/Dummy.h"
#include "decomposer/TreeDecomposer.h"
#include "decomposer/GraphMl.h"

#include "solver/dummy/SolverFactory.h"
#include "solver/clasp/SolverFactory.h"
#include "solver/asp/SolverFactory.h"

#include "printer/Quiet.h"
#include "printer/Progress.h"
#include "printer/DebugHumanReadable.h"
#include "printer/DebugMachineReadable.h"
#include "printer/CountRows.h"

#include "parser/Driver.h"

const std::string Application::MODULE_SECTION = "Module selection";

namespace {
	int strToInt(const std::string& str, const std::string& errorMsg)
	{
		int value;
		size_t idx;
		try {
			value = std::stoi(str, &idx);
			if(idx != str.size())
				throw std::invalid_argument("");
		} catch(const std::invalid_argument&) {
			throw std::runtime_error(errorMsg);
		}
		return value;
	}
}

Application::Application(const std::string& binaryName)
	: binaryName(binaryName)
	, optDecomposer("d", "decomposer", "Use decomposition method <decomposer>")
	, optSolver("s", "solver", "Use <solver> to compute partial solutions")
	, optPrinter("output", "module", "Print information during the run using <module>")
	, optNoCounting("no-counting", "Do not count the number of solutions")
	, optNoOptimization("no-optimization", "Ignore solution costs")
	, optNoPruning("no-pruning", "Prune rejecting subtrees only in the decomposition root")
	, optPrintDecomposition("print-decomposition", "Print the generated decomposition")
	, optPrintProvisional("print-provisional", "Report possibly non-optimal solutions")
	, decomposer(0)
	, solverFactory(0)
	, depth(std::numeric_limits<unsigned int>::max())
{
}

int Application::run(int argc, const char* const* const argv)
{
	// Set up general options
	options::Option optHelp("h", "Print usage information and exit");
	opts.addOption(optHelp);
	// Register an observer for printing the usage when -h is supplied.
	// Note that we use an observer for this instead of checking manually (right after opts.parse()) if optHelp was used, because other observers might perform actions which are undesired if -h has been passed. This is why helpObserver is the first observer we register.
	options::HelpObserver helpObserver(*this, optHelp);
	opts.registerObserver(helpObserver);

	options::Option optVersion("version", "Print version number and exit");
	opts.addOption(optVersion);
	options::VersionObserver versionObserver(*this, optVersion);
	opts.registerObserver(versionObserver);

	options::SingleValueOption optDepth("depth", "d", "Print only item sets of depth at most <d>");
	opts.addOption(optDepth);

	options::MultiValueOption optEdge("e", "edge", "Predicate <edge> declares (hyper)edges");
	opts.addOption(optEdge);

	options::SingleValueOption optInputFile("f", "file", "Read problem instance from <file> (stdin by default)");
	opts.addOption(optInputFile);

	opts.addOption(optNoCounting);
	opts.addOption(optNoOptimization);
	opts.addOption(optNoPruning);
	opts.addOption(optPrintDecomposition);
	opts.addOption(optPrintProvisional);
	options::SingleValueOption optGraphMlOut("graphml-out", "file", "Write the decomposition in the GraphML format to <file>");
	opts.addOption(optGraphMlOut);

	options::SingleValueOption optSeed("seed", "n", "Initialize random number generator with seed <n>");
	opts.addOption(optSeed);

	// Set up module selection options
	opts.addOption(optDecomposer, MODULE_SECTION);
	decomposer::Dummy dummyDecomposer(*this);
	decomposer::TreeDecomposer treeDecomposer(*this, true);
	decomposer::GraphMl graphMlDecomposer(*this);

	opts.addOption(optSolver, MODULE_SECTION);
	solver::dummy::SolverFactory dummySolverFactory(*this);
	solver::clasp::SolverFactory claspSolverFactory(*this, true);
	solver::asp::SolverFactory aspSolverFactory(*this);

	opts.addOption(optPrinter, MODULE_SECTION);
	printer::Quiet quietPrinter(*this);
	printer::Progress progressPrinter(*this, true);
	printer::DebugHumanReadable humanReadableDebugPrinter(*this);
	printer::DebugMachineReadable machineReadableDebugPrinter(*this);
	printer::CountRows countRows(*this);

	time_t seed = time(0);
	// Parse command line
	try {
		opts.parse(argc, argv);
		opts.checkConditions();

		if(optSeed.isUsed())
			seed = strToInt(optSeed.getValue(), "Invalid random seed");

		if(optDepth.isUsed())
			depth = strToInt(optDepth.getValue(), "Invalid depth");

		if(!optEdge.isUsed())
			throw std::runtime_error("Option -e must be supplied at least once");
	}
	catch(...) {
		printUsage();
		throw;
	}

	srand(seed);

	assert(decomposer);
	assert(solverFactory);

	// Get (hyper-)edge predicate names
	parser::Driver::Predicates edgePredicates(optEdge.getValues().begin(), optEdge.getValues().end());

	// Parse instance
	instance = parser::Driver(optInputFile.getValue(), edgePredicates).parse();

	// Decompose instance
	DecompositionPtr decomposition = decomposer->decompose(instance);
	if(optGraphMlOut.isUsed()) {
		std::ofstream graphMlFile(optGraphMlOut.getValue().c_str());
		decomposition->printGraphMl(graphMlFile);
		if(!graphMlFile)
			throw std::runtime_error("Could not write GraphML output");
	}
	printer->decomposerResult(*decomposition);

	// Solve
	ItemTreePtr rootItree = decomposition->getSolver().compute();
	printer->result(rootItree);
	return rootItree ? 10 : 20;
}

void Application::printUsage() const
{
	printVersion();
	std::cout << "Usage: " << binaryName << " [options] < instance" << std::endl;
	opts.printHelp(std::cout);
}

void Application::printVersion() const
{
	std::cout << "D-FLAT version " VERSION_NUMBER << std::endl;
}

const Instance& Application::getInstance() const
{
	return instance;
}

options::OptionHandler& Application::getOptionHandler()
{
	return opts;
}

options::Choice& Application::getDecomposerChoice()
{
	return optDecomposer;
}

options::Choice& Application::getSolverChoice()
{
	return optSolver;
}

options::Choice& Application::getPrinterChoice()
{
	return optPrinter;
}

const SolverFactory& Application::getSolverFactory() const
{
	assert(solverFactory);
	return *solverFactory;
}

Printer& Application::getPrinter() const
{
	assert(printer);
	return *printer;
}

void Application::setDecomposer(Decomposer& d)
{
	decomposer = &d;
}

void Application::setSolverFactory(SolverFactory& s)
{
	solverFactory = &s;
}

void Application::setPrinter(Printer& p)
{
	printer = &p;
}

bool Application::isCountingDisabled() const
{
	return optNoCounting.isUsed();
}

bool Application::isOptimizationDisabled() const
{
	return optNoOptimization.isUsed();
}

bool Application::isPruningDisabled() const
{
	return optNoPruning.isUsed();
}

bool Application::printDecomposition() const
{
	return optPrintDecomposition.isUsed();
}

bool Application::printProvisionalSolutions() const
{
	return optPrintProvisional.isUsed();
}

unsigned int Application::getMaterializationDepth() const
{
	return depth;
}
