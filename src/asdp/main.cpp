#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "ClaspAlgorithm.h"

namespace {
	const int CONSISTENT = 10;
	const int INCONSISTENT = 20;

	void usage(const char* program) {
		const int w = 20;
		std::cerr << "Usage: " << program << " -e hyperedge_pred [...] [-l level] [-n normalization] [--only-decompose] [-p problem_type] [-s seed] [--stats] exchange_program < instance" << std::endl;
		std::cerr << std::endl << std::left;
		std::cerr << '\t' << std::setw(w) << "-e hyperedge_pred: " << "Name of a predicate that declares hyperedges (must be specified at least once)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-l level: " << "Level on polynomial hierarchy; determines depth of tuple assignment tree. Default: 0" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-n normalization: " << "Either \"semi\" (default) or \"normalized\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--only-decompose: " << "Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-p problem_type: " << "Either \"enumeration\" (default), \"counting\" or \"decision\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-s seed: " << "Initialize random number generator with <seed>" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--stats: " << "Print statistics" << std::endl;
		std::cerr << '\t' << std::setw(w) << "exchange_program: " << "File name of the logic program executed in exchange nodes" << std::endl;
		std::cerr << '\t' << std::setw(w) << "instance: " << "File name of the set of facts representing an instance" << std::endl;
		std::cerr << std::endl;
		std::cerr << "Exit code " << CONSISTENT << " means consistent, " << INCONSISTENT << " means inconsistent." << std::endl;
		exit(1);
	}

	inline void printDecompositionStats(sharp::ExtendedHypertree& d) {
		const int w = 20;
		std::cout << std::setw(w) << "Width: " << d.getTreeWidth() << std::endl;
		std::cout << std::setw(w) << "Nodes: " << d.getNumberOfDescendants() << std::endl;
		std::cout << std::setw(w) << "Join nodes: " << d.getNumberOfJoinNodes() << " (" << (100*d.getJoinNodePercentage()) << " %)" << std::endl;
		std::cout << std::setw(w) << "Leafs: " << d.getNumberOfLeafNodes() << std::endl;
	}
}

int main(int argc, char** argv)
{
	unsigned int level = 0;
	enum { ENUMERATION, COUNTING, DECISION } problemType = ENUMERATION;
	time_t seed = time(0);
	sharp::NormalizationType normalizationType = sharp::SemiNormalization;
	bool onlyDecompose = false;
	bool stats = false;
	const char* exchangeProgram = 0;
	std::set<std::string> hyperedgePredicateNames;

	for(int i = 1; i < argc; ++i) {
		bool hasArg = i+1 < argc;
		std::string arg = argv[i];

		if(arg == "-e" && hasArg)
			hyperedgePredicateNames.insert(argv[++i]);
		else if(arg == "-n" && hasArg) {
			std::string typeArg = argv[++i];
			if(typeArg == "semi")
				normalizationType = sharp::SemiNormalization;
			else if(typeArg == "normalized")
				normalizationType = sharp::DefaultNormalization;
			else
				usage(argv[0]);
		}
		else if(arg == "-l" && hasArg) {
			char* endptr;
			level = strtol(argv[++i], &endptr, 0);
			if(*endptr) {
				std::cerr << "Invalid level" << std::endl;
				usage(argv[0]);
			}
		}
		else if(arg == "--only-decompose")
			onlyDecompose = true;
		else if(arg == "-p" && hasArg) {
			std::string typeArg = argv[++i];
			if(typeArg == "enumeration")
				problemType = ENUMERATION;
			else if(typeArg == "counting")
				problemType = COUNTING;
			else if(typeArg == "decision")
				problemType = DECISION;
			else
				usage(argv[0]);
		}
		else if(arg == "-s" && hasArg) {
			char* endptr;
			seed = strtol(argv[++i], &endptr, 0);
			if(*endptr) {
				std::cerr << "Invalid seed" << std::endl;
				usage(argv[0]);
			}
		}
		else if(arg == "--stats")
			stats = true;
		else {
			if(exchangeProgram)
				usage(argv[0]);
			else
				exchangeProgram = argv[i];
		}
	}

	if(!exchangeProgram || hyperedgePredicateNames.empty())
		usage(argv[0]);

	srand(seed);

	// Store all of stdin in a string
	std::ostringstream inputStringStream;
	inputStringStream << std::cin.rdbuf();
	std::string inputString = inputStringStream.str();

	asdp::Problem problem(inputString, hyperedgePredicateNames);

	sharp::ExtendedHypertree* decomposition = problem.calculateHypertreeDecomposition();

	if(stats) {
		std::cout << "Decomposition stats:" << std::endl;
		printDecompositionStats(*decomposition);
		if(normalizationType == sharp::DefaultNormalization) {
			std::cout << "Normalization stats:" << std::endl;
			printDecompositionStats(*decomposition->normalize(sharp::DefaultNormalization));
		} else {
			assert(normalizationType == sharp::SemiNormalization);
			std::cout << "Semi-normalization stats:" << std::endl;
			printDecompositionStats(*decomposition->normalize(sharp::SemiNormalization));
		}
	}
	if(onlyDecompose)
		return 0;

	asdp::ClaspAlgorithm algorithm(problem, exchangeProgram, inputString, normalizationType, level);
	sharp::Solution* solution = problem.calculateSolutionFromDecomposition(&algorithm, decomposition);

	// Print solution
	if(solution) {
		switch(problemType) {
			case ENUMERATION: {
				sharp::EnumerationSolutionContent* sol = dynamic_cast<sharp::EnumerationSolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::EnumerationSolutionContent>()));

				std::cout << "Solutions: " << sol->enumerations.size() << std::endl;
				std::cout << "{";
				std::string osep = "";
				foreach(const sharp::VertexSet& o, sol->enumerations) {
					std::cout << osep << "{";
					std::string isep = "";
					foreach(sharp::Vertex i, o) {
						std::cout << isep << problem.getVertexName(i);
						isep = ",";
					}
					std::cout << "}" << std::flush;
					osep = ",";
				}
				std::cout << "}" << std::endl;
				return sol->enumerations.empty() ? INCONSISTENT : CONSISTENT;
			} break;

			case COUNTING: {
				sharp::CountingSolutionContent* content = dynamic_cast<sharp::CountingSolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::CountingSolutionContent>()));
				std::cout << "Solutions: " << content->count << std::endl;
				return content->count == 0 ? INCONSISTENT : CONSISTENT;
			} break;

			case DECISION: {
				sharp::ConsistencySolutionContent* content = dynamic_cast<sharp::ConsistencySolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::ConsistencySolutionContent>()));
				if(content->consistent == false) {
					std::cout << "INCONSISTENT" << std::endl;
					return INCONSISTENT;
				}
				std::cout << "CONSISTENT" << std::endl;
				return CONSISTENT;
			} break;
		}
	}
	return 0;
}
