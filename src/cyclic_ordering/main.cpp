#include <iostream>
#include <iomanip>
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
		std::cerr << "Usage: " << program << " [--only-decompose] [-s seed] [--stats]" << std::endl;
		std::cerr << std::endl << std::left;
		std::cerr << '\t' << std::setw(w) << "-n normalization: " << "Either \"semi\" (default) or \"normalized\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--only-decompose: " << "Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-s seed: " << "Initialize random number generator with <seed>" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--stats: " << "Print statistics" << std::endl;
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
	sharp::NormalizationType normalizationType = sharp::SemiNormalization;
	time_t seed = time(0);
	bool onlyDecompose = false;
	bool stats = false;

	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if(arg == "-n") {
			std::string typeArg = argv[++i];
			if(typeArg == "semi")
				normalizationType = sharp::SemiNormalization;
			else if(typeArg == "normalized")
				normalizationType = sharp::DefaultNormalization;
			else
				usage(argv[0]);
		}
		else if(arg == "--only-decompose")
			onlyDecompose = true;
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

	cyclic_ordering::Problem problem(std::cin);

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

	sharp::Solution* solution;
	cyclic_ordering::ClaspAlgorithm algorithm(problem, "asp_encodings/cyclic_ordering/exchange_decision.lp", normalizationType);
	solution = problem.calculateSolutionFromDecomposition(&algorithm, decomposition);

	// Print solution
	if(solution) {
		sharp::ConsistencySolutionContent* content = dynamic_cast<sharp::ConsistencySolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::ConsistencySolutionContent>()));
		if(content->consistent == false) {
			std::cout << "INCONSISTENT" << std::endl;
			return INCONSISTENT;
		}
		std::cout << "CONSISTENT" << std::endl;
		return CONSISTENT;
	}
	assert(false);
	return 0;
}
