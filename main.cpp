#include <iostream>
#include <iomanip>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "Algorithm.h"

namespace {
	const int CONSISTENT = 0;
	const int INCONSISTENT = 23;

	inline void usage(const char* program) {
		std::cerr << "Usage: " << program << "[--only-decompose] [-p problem_type] [-s seed]" << std::endl;
		std::cerr << "--only-decompose: Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		std::cerr << "-p problem_type: Either enumeration (default), counting or decision" << std::endl;
		std::cerr << "-s seed: Initialize random number generator with <seed>" << std::endl;
		std::cerr << "--stats: Print statistics" << std::endl;
		std::cerr << "If \"problem_type\" is \"decision\", exit code " << CONSISTENT << " means consistent, " << INCONSISTENT << " means inconsistent" << std::endl;
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
	Algorithm::ProblemType problemType = Algorithm::ENUMERATION;
	time_t seed = time(0);
	bool onlyDecompose = false;
	bool stats = false;

	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if(arg == "--only-decompose")
			onlyDecompose = true;
		else if(arg == "-p") {
			std::string typeArg = argv[++i];
			if(typeArg == "enumeration")
				problemType = Algorithm::ENUMERATION;
			else if(typeArg == "counting")
				problemType = Algorithm::COUNTING;
			else if(typeArg == "decision")
				problemType = Algorithm::DECISION;
			else
				usage(argv[0]);
		}
		else if(arg == "-s") {
			char* endptr;
			seed = strtol(argv[++i], &endptr, 0);
			if(*endptr) {
				std::cerr << "Invalid seed" << std::endl;
				usage(argv[0]);
			}
		}
		else if(arg == "--stats")
			stats = true;
		else
			usage(argv[0]);
	}

	srand(seed);

	Problem problem(std::cin, true);

	//sharp::Solution* solution = problem.calculateSolution(&algorithm);
	sharp::ExtendedHypertree* decomposition = problem.calculateHypertreeDecomposition();

	if(stats) {
		std::cout << "Decomposition stats:" << std::endl;
		printDecompositionStats(*decomposition);
		std::cout << "Semi-normalization stats:" << std::endl;
		printDecompositionStats(*decomposition->normalize(sharp::SemiNormalization));
//		std::cout << "Normalization stats:" << std::endl;
//		printDecompositionStats(*decomposition->normalize(sharp::DefaultNormalization));
	}
	if(onlyDecompose)
		return 0;

	Algorithm algorithm(problem, problemType);
	sharp::Solution* solution = problem.calculateSolutionFromDecomposition(&algorithm, decomposition);

	// Print solution
	if(solution) {
		switch(problemType) {
			case Algorithm::ENUMERATION: {
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
			} break;

			case Algorithm::COUNTING: {
				sharp::CountingSolutionContent* content = dynamic_cast<sharp::CountingSolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::CountingSolutionContent>()));
				std::cout << "Solutions: " << content->count << std::endl;
			} break;

			case Algorithm::DECISION: {
				sharp::ConsistencySolutionContent* content = dynamic_cast<sharp::ConsistencySolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::ConsistencySolutionContent>()));
				if(content->consistent == false) {
					std::cout << "INCONSISTENT";
					return INCONSISTENT;
				}
				std::cout << "CONSISTENT" << std::endl;
				return CONSISTENT;
			} break;
		}
	}
	return 0;
}
