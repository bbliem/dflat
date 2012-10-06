#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sharp/main>

#include "Problem.h"
#include "ClaspAlgorithm.h"
#include "solution/DecisionPlan.h"
#include "solution/DecisionSolution.h"
#include "solution/CountingPlan.h"
#include "solution/CountingSolution.h"
#include "solution/EnumerationPlan.h"
#include "solution/EnumerationSolution.h"
#include "solution/OptValuePlan.h"
#include "solution/OptValueSolution.h"
#include "solution/OptCountingPlan.h"
#include "solution/OptCountingSolution.h"
#include "solution/OptEnumPlan.h"
#include "solution/OptEnumSolution.h"

namespace {
	const int CONSISTENT = 10;
	const int INCONSISTENT = 20;

	void usage(const char* program) {
		const int w = 20;
		std::cerr << "Usage: " << program << " -e hyperedge_pred [...] [-j join_program] [-l level] [-n normalization] [--only-decompose] [-p problem_type] [-s seed] [--stats] -x exchange_program < instance" << std::endl;
		std::cerr << std::endl << std::left;
		std::cerr << '\t' << std::setw(w) << "-e hyperedge_pred: " << "Name of a predicate that declares hyperedges (must be specified at least once)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-j join_program: " << "File name of the logic program executed in join nodes (if omitted join equal tuples)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-l level: " << "Level on polynomial hierarchy; determines depth of tuple assignment tree. Default: 0" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-n normalization: " << "Either \"semi\" (default) or \"normalized\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--only-decompose: " << "Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-p problem_type: " << "Either \"enumeration\" (default), \"counting\", \"decision\", \"opt-enum\", \"opt-counting\" or \"opt-value\"" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-s seed: " << "Initialize random number generator with <seed>" << std::endl;
		std::cerr << '\t' << std::setw(w) << "--stats: " << "Print statistics" << std::endl;
		std::cerr << '\t' << std::setw(w) << "-x exchange_program: " << "File name of the logic program executed in exchange nodes" << std::endl;
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
	enum { ENUMERATION, COUNTING, DECISION, OPT_ENUM, OPT_COUNTING, OPT_VALUE } problemType = ENUMERATION;
	time_t seed = time(0);
	sharp::NormalizationType normalizationType = sharp::SemiNormalization;
	bool onlyDecompose = false;
	bool stats = false;
	const char* exchangeProgram = 0;
	const char* joinProgram = 0;
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
		else if(arg == "-j" && hasArg) {
			if(joinProgram)
				usage(argv[0]);
			else
				joinProgram = argv[++i];
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
			else if(typeArg == "opt-enum")
				problemType = OPT_ENUM;
			else if(typeArg == "opt-counting")
				problemType = OPT_COUNTING;
			else if(typeArg == "opt-value")
				problemType = OPT_VALUE;
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
		else if(arg == "-x" && hasArg) {
			if(exchangeProgram)
				usage(argv[0]);
			else
				exchangeProgram = argv[++i];
		}
		else
			usage(argv[0]);
	}

	if((!exchangeProgram && !onlyDecompose) || hyperedgePredicateNames.empty())
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

	switch(problemType) {
		case ENUMERATION: {
			sharp::GenericPlanFactory<asdp::solution::EnumerationPlan, Tuple> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				std::auto_ptr<asdp::solution::EnumerationSolution> s(dynamic_cast<asdp::solution::EnumerationSolution*>(plan->materialize()));
				delete plan;
				std::cout << "Solutions: " << s->getSolutions().size() << std::endl;

				foreach(const Tuple::Assignment& a, s->getSolutions()) {
					foreach(const Tuple::Assignment::value_type& pair, a)
						std::cout << pair.first << '=' << pair.second << ' ';
					std::cout << std::endl;
				}

				return s->getSolutions().empty() ? INCONSISTENT : CONSISTENT;
			} else {
				std::cout << "Solutions: 0" << std::endl;
				return INCONSISTENT;
			}
		} break;

		case COUNTING: {
			sharp::GenericPlanFactory<asdp::solution::CountingPlan> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				std::auto_ptr<asdp::solution::CountingSolution> s(dynamic_cast<asdp::solution::CountingSolution*>(plan->materialize()));
				delete plan;
				std::cout << "Solutions: " << s->getCount() << std::endl;
				return s->getCount() == 0 ? INCONSISTENT : CONSISTENT;
			} else {
				std::cout << "Solutions: 0" << std::endl;
				return INCONSISTENT;
			}
		} break;

		case DECISION: {
			sharp::GenericPlanFactory<asdp::solution::DecisionPlan> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				// Since there is a plan, there must also be a solution
#ifndef NDEBUG
				std::auto_ptr<asdp::solution::DecisionSolution> s(dynamic_cast<asdp::solution::DecisionSolution*>(plan->materialize()));
				assert(s.get());
#endif
				delete plan;
				std::cout << "CONSISTENT" << std::endl;
				return CONSISTENT;
			} else {
				std::cout << "INCONSISTENT" << std::endl;
				return INCONSISTENT;
			}
		} break;

		case OPT_ENUM: {
			sharp::GenericPlanFactory<asdp::solution::OptEnumPlan, Tuple> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				std::auto_ptr<asdp::solution::OptEnumSolution> s(dynamic_cast<asdp::solution::OptEnumSolution*>(plan->materialize()));
				delete plan;

				std::cout << "Minimum cost: " << s->getCost() << std::endl;
				std::cout << "Optimal solutions: " << s->getSolutions().size() << std::endl;

				foreach(const Tuple::Assignment& a, s->getSolutions()) {
					foreach(const Tuple::Assignment::value_type& pair, a)
						std::cout << pair.first << '=' << pair.second << ' ';
					std::cout << std::endl;
				}

				return s->getSolutions().empty() ? INCONSISTENT : CONSISTENT;
			} else {
				std::cout << "Optimal solutions: 0" << std::endl;
				return INCONSISTENT;
			}
		} break;

		case OPT_COUNTING: {
			sharp::GenericPlanFactory<asdp::solution::OptCountingPlan, Tuple> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				std::auto_ptr<asdp::solution::OptCountingSolution> s(dynamic_cast<asdp::solution::OptCountingSolution*>(plan->materialize()));
				delete plan;
				std::cout << "Minimum cost: " << s->getCost() << std::endl;
				std::cout << "Optimal solutions: " << s->getCount() << std::endl;
				return s->getCount() == 0 ? INCONSISTENT : CONSISTENT;
			} else {
				std::cout << "Optimal Solutions: 0" << std::endl;
				return INCONSISTENT;
			}
		} break;

		case OPT_VALUE: {
			sharp::GenericPlanFactory<asdp::solution::OptValuePlan, Tuple> planFactory;
			asdp::ClaspAlgorithm algorithm(problem, planFactory, inputString, exchangeProgram, joinProgram, normalizationType, level);
			sharp::Plan* plan = problem.calculatePlanFromDecomposition(&algorithm, decomposition);

			if(plan) {
				std::auto_ptr<asdp::solution::OptValueSolution> s(dynamic_cast<asdp::solution::OptValueSolution*>(plan->materialize()));
				delete plan;
				std::cout << "Minimum cost: " << s->getCost() << std::endl;
				return CONSISTENT;
			} else {
				std::cout << "INCONSISTENT" << std::endl;
				return INCONSISTENT;
			}
		} break;
	}
	assert(false); // Should never reach this point
	return INCONSISTENT;
}
