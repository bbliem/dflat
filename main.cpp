#include <iostream>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "Algorithm.h"

namespace {
	void usage(const char* program) {
		std::cerr << "Usage: " << program << " [-s seed] [-p problem_type]" << std::endl;
		std::cerr << "-s seed: Initialize random number generator with <seed>" << std::endl;
		std::cerr << "-t problem_type: Either enumeration (default), counting or decision" << std::endl;
		exit(1);
	}
}

int main(int argc, char** argv)
{
	Algorithm::ProblemType problemType = Algorithm::ENUMERATION;
	time_t seed = time(0);

	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if(arg == "-p") {
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
		else
			usage(argv[0]);
	}

	srand(seed);

	Problem problem(std::cin, true);
	Algorithm algorithm(problem, problemType);

	sharp::Solution* solution = problem.calculateSolution(&algorithm);

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
				if(content->consistent == false)
					std::cout << "not ";
				std::cout << "consistent" << std::endl;
			} break;
		}
	}
	return 0;
}
