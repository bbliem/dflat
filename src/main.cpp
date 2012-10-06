/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <gmpxx.h>
#include <sharp/main>

#include "Problem.h"
#include "NonNormalizedAlgorithm.h"
#include "SemiNormalizedAlgorithm.h"
#include "EnumerationIterator.h"

namespace {
	const int CONSISTENT = 10;
	const int INCONSISTENT = 20;

	void usage(const char* program) {
		const int w = 22;
		std::cerr << "Usage: " << program << " -e hyperedge_pred [...] [-j join_program] [--multi-level] [-n normalization] [--only-decompose] [-p problem_type] [-s seed] [--stats] [-x exchange_program] [program] < instance" << std::endl;
		std::cerr << std::endl << std::left;
		std::cerr << "  " << std::setw(w) << "-e hyperedge_pred: " << "Name of a predicate that declares hyperedges (must be specified at least once)" << std::endl;
		std::cerr << "  " << std::setw(w) << "-j join_program: " << "File name of the logic program executed in join nodes (if omitted join equal rows)" << std::endl;
		std::cerr << "  " << std::setw(w) << "--multi-level: " << "Use multi-level item sets" << std::endl;
		std::cerr << "  " << std::setw(w) << "-n normalization: " << "Either \"none\" (default), \"semi\" or \"normalized\"" << std::endl;
		std::cerr << "  " << std::setw(w) << "--only-decompose: " << "Only perform decomposition and do not solve (useful with --stats)" << std::endl;
		// TODO: Add problem types "search" and "opt-search", ideally with incremental solving
		std::cerr << "  " << std::setw(w) << "-p problem_type: " << "Either \"enumeration\" (default), \"counting\", \"decision\", \"opt-enum\", \"opt-counting\" or \"opt-value\"" << std::endl;
		std::cerr << "  " << std::setw(w) << "-s seed: " << "Initialize random number generator with <seed>" << std::endl;
		std::cerr << "  " << std::setw(w) << "--stats: " << "Print statistics" << std::endl;
		std::cerr << "  " << std::setw(w) << "-x exchange_program: " << "File name of the logic program executed in exchange nodes (only for semi-normalized decompositions)" << std::endl;
		std::cerr << "  " << std::setw(w) << "program: " << "File name of the logic program executed in nodes (only for semi-normalized decompositions)" << std::endl;
		std::cerr << "  " << std::setw(w) << "instance: " << "File name of the set of facts representing an instance" << std::endl;
		std::cerr << std::endl;
		std::cerr << "Exit code " << CONSISTENT << " means there is a solution, " << INCONSISTENT << " means there is none." << std::endl;
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
	try {
		bool multiLevel = false;
		enum { ENUMERATION, COUNTING, DECISION, OPT_ENUM, OPT_COUNTING, OPT_VALUE } problemType = ENUMERATION;
		time_t seed = time(0);
		sharp::NormalizationType normalizationType = sharp::NoNormalization;
		bool onlyDecompose = false;
		bool stats = false;
		const char* program = 0;
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
				if(typeArg == "none")
					normalizationType = sharp::NoNormalization;
				else if(typeArg == "semi")
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
			else if(arg == "--multi-level")
				multiLevel = true;
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
			else {
				if(program)
					usage(argv[0]);
				else
					program = argv[i];
			}
		}

		if(!onlyDecompose) {
			if((!exchangeProgram && !program) || (program && exchangeProgram) || 
					(program && joinProgram) || hyperedgePredicateNames.empty() ||
					(exchangeProgram && normalizationType == sharp::NoNormalization))
				usage(argv[0]);
		}

		srand(seed);

		// Store all of stdin in a string
		std::ostringstream inputStringStream;
		inputStringStream << std::cin.rdbuf();
		std::string inputString = inputStringStream.str();

		Problem problem(inputString, hyperedgePredicateNames);

		sharp::ExtendedHypertree* decomposition = problem.calculateHypertreeDecomposition();

		if(stats) {
			std::cout << "Decomposition stats:" << std::endl;
			printDecompositionStats(*decomposition);
			if(normalizationType == sharp::DefaultNormalization) {
				std::cout << "Normalization stats:" << std::endl;
				printDecompositionStats(*decomposition->normalize(sharp::DefaultNormalization));
			} else if(normalizationType == sharp::SemiNormalization) {
				assert(normalizationType == sharp::SemiNormalization);
				std::cout << "Semi-normalization stats:" << std::endl;
				printDecompositionStats(*decomposition->normalize(sharp::SemiNormalization));
			}
		}
		if(onlyDecompose)
			return 0;

		bool ignoreOptimization = false;
		switch(problemType) {
			case ENUMERATION:
			case COUNTING:
			case DECISION:
				ignoreOptimization = true;
				break;
			default:
				break;
		}

		std::auto_ptr<Algorithm> algorithm;
		if(exchangeProgram)
			algorithm.reset(new SemiNormalizedAlgorithm(problem, inputString, exchangeProgram, joinProgram, normalizationType, ignoreOptimization, multiLevel));
		else
			algorithm.reset(new NonNormalizedAlgorithm(problem, inputString, program, normalizationType, ignoreOptimization, multiLevel));

		std::auto_ptr<sharp::Table> table(problem.calculateTableFromDecomposition(algorithm.get(), decomposition));

#ifdef PROGRESS_REPORT
		std::cout << '\r' << std::setw(66) << std::left << "Done." << std::endl; // Clear/end progress line
#endif

		assert(table.get());

		if(table->empty()) {
			if(problemType == DECISION || problemType == OPT_VALUE)
				std::cout << "NO" << std::endl;
			else
				std::cout << "Solutions: 0" << std::endl;
			return INCONSISTENT;
		}
		
		if(table->size() != 1)
			throw std::runtime_error("Root table must have 0 or 1 rows");

		const Row& row = *dynamic_cast<const Row*>(*table->begin());

		switch(problemType) {
			case ENUMERATION:
			case OPT_ENUM:
				{
					EnumerationIterator it(row);
					mpz_class numSolutions = 0;

					while(it.isValid()) {
						foreach(const std::string& item, *it)
							std::cout << item << ' ';
						std::cout << std::endl;
						++it;
						++numSolutions;
					}

					if(problemType == OPT_ENUM) {
						std::cout << "Minimum cost: " << row.getCost() << std::endl;
						std::cout << "Optimal solutions: " << numSolutions << std::endl;
					} else
						std::cout << "Solutions: " << numSolutions << std::endl;

					return numSolutions == 0 ? INCONSISTENT : CONSISTENT;
				}
				break;

			case COUNTING:
			case OPT_COUNTING:
				{
					if(problemType == OPT_COUNTING) {
						std::cout << "Minimum cost: " << row.getCost() << std::endl;
						std::cout << "Optimal solutions: " << row.getCount() << std::endl;
					} else
						std::cout << "Solutions: " << row.getCount() << std::endl;
					return row.getCount() == 0 ? INCONSISTENT : CONSISTENT;
				}
				break;

			case DECISION:
			case OPT_VALUE:
				{
					if(problemType == OPT_VALUE)
						std::cout << "Minimum cost: " << row.getCost() << std::endl;
					else
						std::cout << "YES" << std::endl;
					return CONSISTENT;
				}
				break;
		}
	} catch(const std::exception& e) {
		std::cerr << std::endl << "Error: " << e.what() << std::endl;
		return 2;
	}

	assert(false); // Should never reach this point
	return INCONSISTENT;
}
