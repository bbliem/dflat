#include <iostream>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "Algorithm.h"

int main(int argc, char** argv)
{
	bool enumerate = true;
	time_t seed = time(0);

	for(int i = 1; i < argc; ++i) {
		std::string arg = argv[i];

		if(arg == "-c")
			enumerate = false;
		else if(arg == "-s") {
			char* endptr;
			seed = strtol(argv[++i], &endptr, 0);
			if(*endptr) {
				std::cerr << "Invalid seed" << std::endl;
				return 2;
			}
		}
		else {
			std::cerr << "Usage: " << argv[0] << " [-s seed] [-c]" << std::endl;
			std::cerr << "-s seed: Initialize random number generator with <seed>" << std::endl;
			std::cerr << "-c: Don't enumerate answer sets, just count them" << std::endl;
			return 1;
		}
	}

	srand(seed);

	Problem problem(std::cin, true);
	Algorithm algorithm(problem);

	sharp::Solution* solution = problem.calculateSolution(&algorithm);

	// Print solution
	if(solution) {
		if(enumerate) {
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
		} else {
			sharp::CountingSolutionContent* content = dynamic_cast<sharp::CountingSolutionContent*>(solution->getContent(new sharp::GenericInstantiator<sharp::CountingSolutionContent>()));
			std::cout << "Solutions: " << content->count << std::endl;
		}
	}
	return 0;
}
