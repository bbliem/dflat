/*{{{
Copyright 2012-2014, Bernhard Bliem
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
#include <sstream>
#include <cassert>

#include "Application.h"
#include "Decomposition.h"

#include "options/MultiValueOption.h"
#include "options/SingleValueOption.h"
#include "options/OptionHandler.h"
#include "options/HelpObserver.h"

#include "decomposer/Dummy.h"
#include "decomposer/TreeDecomposer.h"

#include "solver/dummy/SolverFactory.h"
#include "solver/asp/SolverFactory.h"

#include "printer/Quiet.h"
#include "printer/Progress.h"
#include "printer/DebugHumanReadable.h"
#include "printer/DebugMachineReadable.h"

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
	, optNoPruning("no-pruning", "Prune rejecting subtrees only in the decomposition root")
	, optPrintDecomposition("print-decomposition", "Print the generated decomposition")
	, decomposer(0)
	, solverFactory(0)
	, depth(std::numeric_limits<unsigned int>::max())
{
}

int Application::run(int argc, char** argv)
{
	// Set up general options
	options::Option optHelp("h", "Print usage information and exit");
	opts.addOption(optHelp);
	// Register an observer for printing the usage when -h is supplied.
	// Note that we use an observer for this instead of checking manually (right after opts.parse()) if optHelp was used, because other observers might perform actions which are undesired if -h has been passed. This is why helpObserver is the first observer we register.
	options::HelpObserver helpObserver(*this, optHelp);
	opts.registerObserver(helpObserver);

	options::SingleValueOption optDepth("depth", "d", "Print only item sets of depth at most <d>");
	opts.addOption(optDepth);

	options::MultiValueOption optEdge("e", "edge", "Predicate <edge> declares (hyper)edges");
	opts.addOption(optEdge);

	opts.addOption(optNoCounting);
	opts.addOption(optNoPruning);
	opts.addOption(optPrintDecomposition);

	options::SingleValueOption optSeed("seed", "n", "Initialize random number generator with seed <n>");
	opts.addOption(optSeed);

	// Set up module selection options
	opts.addOption(optDecomposer, MODULE_SECTION);
	decomposer::Dummy dummyDecomposer(*this);
	decomposer::TreeDecomposer treeDecomposer(*this, true);

	opts.addOption(optSolver, MODULE_SECTION);
	solver::dummy::SolverFactory dummySolverFactory(*this);
	solver::asp::SolverFactory aspSolverFactory(*this, true);

	opts.addOption(optPrinter, MODULE_SECTION);
	printer::Quiet quietPrinter(*this);
	printer::Progress progressPrinter(*this, true);
	printer::DebugHumanReadable humanReadableDebugPrinter(*this);
	printer::DebugMachineReadable machineReadableDebugPrinter(*this);

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
		usage();
		throw;
	}

	srand(seed);

	assert(decomposer);
	assert(solverFactory);

	// Get (hyper-)edge predicate names
	parser::Driver::Predicates edgePredicates(optEdge.getValues().begin(), optEdge.getValues().end());

	// Store all of stdin in a string
	std::ostringstream inputStringStream;
	inputStringStream << std::cin.rdbuf();
	inputString = inputStringStream.str();

	// Parse instance
	inputHypergraph = parser::Driver(inputString, edgePredicates).parse();

	// Decompose instance
	DecompositionPtr decomposition = decomposer->decompose(inputHypergraph);
	printer->decomposerResult(*decomposition);

	// Solve
	ItemTreePtr rootItree = decomposition->getSolver().compute();
	printer->result(rootItree);
	return rootItree ? 10 : 20;
}

void Application::usage() const
{
	std::cerr << "Usage: " << binaryName << " [options] < instance" << std::endl;
	opts.printHelp();
}

const std::string& Application::getInputString() const
{
	return inputString;
}

const Hypergraph& Application::getInputHypergraph() const
{
	return inputHypergraph;
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

bool Application::isPruningDisabled() const
{
	return optNoPruning.isUsed();
}

bool Application::printDecomposition() const
{
	return optPrintDecomposition.isUsed();
}

unsigned int Application::getMaterializationDepth() const
{
	return depth;
}
