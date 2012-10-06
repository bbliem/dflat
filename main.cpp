#include <iostream>
#include <iomanip>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "Algorithm.h"
#include "AlgorithmNormalized.h"

namespace {
	const int CONSISTENT = 0;
	const int INCONSISTENT = 23;

	void usage(const char* program) {
		const int w = 20;
		std::cerr << "Usage: " << program << " [-a algorithm] [-n normalization] [--only-decompose] [-p problem_type] [-s seed]" << std::endl;
		std::cerr << std::endl << std::left;
		std::cerr << '\t' << std::setw(w) << "-a algorithm: " << "Either \"semi\" or \"normalized\" (default: argument of -n)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-n normalization: " << "Either \"semi\" (default) or \"normalized\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--only-decompose: " << "Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-p problem_type: " << "Either \"enumeration\" (default), \"counting\" or \"decision\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-s seed: " << "Initialize random number generator with <seed>" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--stats: " << "Print statistics" << std::endl;
		std::cerr << std::endl;
		std::cerr << "\"-a normalized\" only works if \"-n normalized\" is set." << std::endl;
		std::cerr << "If \"problem_type\" is \"decision\", exit code " << CONSISTENT << " means consistent, " << INCONSISTENT << " means inconsistent." << std::endl;
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
	bool algorithmSpecified = false;
	sharp::NormalizationType algorithmType = sharp::SemiNormalization;
	sharp::NormalizationType normalizationType = sharp::SemiNormalization;
	bool onlyDecompose = false;
	bool stats = false;

	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if(arg == "-a") {
			algorithmSpecified = true;
			std::string typeArg = argv[++i];
			if(typeArg == "semi")
				algorithmType = sharp::SemiNormalization;
			else if(typeArg == "normalized")
				algorithmType = sharp::DefaultNormalization;
			else
				usage(argv[0]);
		}
		else if(arg == "-n") {
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

	if(algorithmSpecified) {
		if(algorithmType == sharp::DefaultNormalization && normalizationType == sharp::SemiNormalization)
			usage(argv[0]);
	} else
		algorithmType = normalizationType;

	srand(seed);

	Problem problem(std::cin, true);

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
	if(normalizationType == sharp::DefaultNormalization) {
		AlgorithmNormalized algorithm(problem, problemType, algorithmType == sharp::SemiNormalization);
		solution = problem.calculateSolutionFromDecomposition(&algorithm, decomposition);
	} else {
		assert(normalizationType == sharp::SemiNormalization);
		Algorithm algorithm(problem, problemType);
		solution = problem.calculateSolutionFromDecomposition(&algorithm, decomposition);
	}

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
