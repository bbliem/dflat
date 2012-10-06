#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <gringo/streams.h>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#if PROGRESS_REPORT > 2
#include <boost/timer/timer.hpp>
#endif

#include "Algorithm.h"
#include "Problem.h"
#include "Tuple.h"
#include "PowerSet.h"
#include "GringoOutputProcessor.h"
#include "ClaspInputReader.h"
#include "ModelProcessor.h"

using sharp::Solution;
using sharp::TupleSet;
using sharp::ExtendedHypertree;
using sharp::VertexSet;
using sharp::Vertex;

namespace {
	inline void describeRule(std::stringstream& ss, Problem& problem, const sharp::Vertex& rule)
	{
		ss << "rule(v" << rule << ")." << std::endl;

		const Problem::VerticesInRule& atoms = problem.getAtomsInRule(rule);
		foreach(Vertex v, atoms.head)
			ss << "head(v" << rule << ",v" << v << ")." << std::endl;
		foreach(Vertex v, atoms.pos)
			ss << "pos(v" << rule << ",v" << v << ")." << std::endl;
		foreach(Vertex v, atoms.neg)
			ss << "neg(v" << rule << ",v" << v << ")." << std::endl;
	}

	inline void describeChildTuples(std::stringstream& ss, const TupleSet& tuples) {
		foreach(const TupleSet::value_type& tupleAndSolution, tuples) {
			const Tuple& tuple = *dynamic_cast<Tuple*>(tupleAndSolution.first);

			const TupleSet::value_type* mi = &tupleAndSolution; // An evil trick...

			ss << "oldMi(m" << mi << ")." << std::endl;

			foreach(Vertex v, tuple.atoms)
				ss << "oldMAtom(m" << mi << ",v" << v << ")." << std::endl;
			foreach(Vertex v, tuple.rules)
				ss << "oldMRule(m" << mi << ",v" << v << ")." << std::endl;
		}
	}

	inline void describeExchangeNodeContents(std::stringstream& ss, const ExtendedHypertree& node, Problem& problem) {
		const VertexSet& currentVertices = node.getVertices();

		// Declare contents of the current bag
		foreach(Vertex v, currentVertices) {
			if(problem.vertexIsRule(v)) {
				ss << "current(v" << v << ")." << std::endl;
				describeRule(ss, problem, v);
			}
			else // an atom
				ss
					<< "current(v" << v << ")." << std::endl
					<< "atom(v" << v << ")." << std::endl;
		}

		// Declare contents of the child bag
		if(node.getType() != sharp::Leaf) {
			const VertexSet& beforeVertices = node.firstChild()->getVertices();

			foreach(Vertex v, beforeVertices) {
				if(problem.vertexIsRule(v)) {
					ss << "before(v" << v << ")." << std::endl;
					describeRule(ss, problem, v);
				}
				else // an atom
					ss
						<< "before(v" << v << ")." << std::endl
						<< "atom(v" << v << ")." << std::endl;
			}
		}
	}
}

Algorithm::Algorithm(Problem& problem, Algorithm::ProblemType problemType, AlgorithmType algorithmType)
	: AbstractSemiNormalizedHTDAlgorithm(&problem), problem(dynamic_cast<Problem&>(problem)), problemType(problemType), algorithmType(algorithmType)
#if PROGRESS_REPORT > 0
	  , nodesProcessed(0)
#endif
{
}

Algorithm::~Algorithm()
{
}

Solution* Algorithm::selectSolution(TupleSet* tuples, const ExtendedHypertree* root)
{
#if PROGRESS_REPORT > 0
	std::cout << '\r' << std::setw(66) << std::left << "Done." << std::endl; // Clear/end progress line
#endif

	Solution* result = createEmptySolution();

	VertexSet currentRules;
	foreach(Vertex v, root->getVertices())
		if(problem.vertexIsRule(v))
			currentRules.insert(v);

	for(TupleSet::iterator it = tuples->begin(); it != tuples->end(); ++it) {
		const Tuple& t = *dynamic_cast<Tuple*>(it->first);

		// A tuple corresponds to a valid solution if all rules in the current bag are in it
		if(t.rules == currentRules)
			result = combineSolutions(sharp::Union, result, it->second);
	}

	return result;
}

TupleSet* Algorithm::evaluateBranchNode(const ExtendedHypertree* node)
{
	TupleSet* left = evaluateNode(node->firstChild());
	TupleSet* right = evaluateNode(node->secondChild());
#if PROGRESS_REPORT > 0
	printProgressLine(node, left->size()+right->size());
#if PROGRESS_REPORT > 2
	boost::timer::auto_cpu_timer timer(" %ts\n");
#endif
#endif
	TupleSet* ts = new TupleSet;

	// TupleSets are ordered, use sort merge join algorithm
	TupleSet::const_iterator lit = left->begin();
	TupleSet::const_iterator rit = right->begin();
#define TUP(X) (*dynamic_cast<const Tuple*>(X->first)) // FIXME: Think of something better
	while(lit != left->end() && rit != right->end()) {
		while(TUP(lit).atoms != TUP(rit).atoms) {
			// Advance iterator pointing to smaller value
			if(TUP(lit).atoms < TUP(rit).atoms) {
				++lit;
				if(lit == left->end())
					goto endJoin;
			} else {
				++rit;
				if(rit == right->end())
					goto endJoin;
			}
		}

		// Now lit and rit join
		// Remember position of rit and advance rit until no more match
		TupleSet::const_iterator mark = rit;
joinLitWithAllPartners:
		do {
			Tuple& ast = *new Tuple;
			ast.atoms = TUP(lit).atoms;
			ast.rules = TUP(lit).rules;
			ast.rules.insert(TUP(rit).rules.begin(), TUP(rit).rules.end());
			Solution *s = combineSolutions(sharp::CrossJoin, lit->second, rit->second);
			// FIXME: It seems this is the same as Algorithm::addToTupleSet. Use that instead?
			std::pair<TupleSet::iterator, bool> result = ts->insert(TupleSet::value_type(&ast, s));
			if(!result.second) {
				Solution *orig = result.first->second;
				ts->erase(result.first);
				ts->insert(TupleSet::value_type(&ast, combineSolutions(sharp::Union, orig, s)));
			}
			++rit;
		} while(rit != right->end() && TUP(lit).atoms == TUP(rit).atoms);

		// lit and rit don't join anymore. Advance lit. If it joins with mark, reset rit to mark.
		++lit;
		if(lit == left->end())
			break;

		if(TUP(lit).atoms == TUP(mark).atoms) {
			rit = mark;
			goto joinLitWithAllPartners; // Ha!
		}
	}
endJoin:

	delete left;
	delete right;

#ifdef VERBOSE
	std::cout << "Join node result:" << std::endl;
	for(TupleSet::const_iterator it = ts->begin(); it != ts->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout, problem);
	std::cout << std::endl;
#endif

	return ts;
}

TupleSet* Algorithm::evaluatePermutationNode(const ExtendedHypertree* node)
{
	// TODO: Make configurable
	const char* aspProgram = 0;
	std::stringstream* bagContents = 0;

	if(algorithmType == SEMI_ASP) {
		aspProgram = problemType == DECISION ? "exchange_decision.lp" : "exchange.lp";
		// Build input for the exchange node program
		bagContents = new std::stringstream;
		describeExchangeNodeContents(*bagContents, *node, problem);
	}

	TupleSet* newTuples = new TupleSet;

	if(node->getType() != sharp::Leaf) {
		TupleSet* childTuples = evaluateNode(node->firstChild());
#if PROGRESS_REPORT > 0
		printProgressLine(node, childTuples->size());
#if PROGRESS_REPORT > 2
		boost::timer::auto_cpu_timer timer(" %ts\n");
#endif
#endif
#ifdef VERBOSE
		printBagContents(node->getVertices());
#endif
		// There might be no child tuples, consider as a child e.g. a join node without matches.
		// If we were to run the program without child tuples, it would consider the current node as a leaf node and wrongly generate new tuples.
		if(childTuples->empty() == false) {
			if(algorithmType == SEMI_ASP) {
				std::stringstream* childTuplesInput = new std::stringstream;
				describeChildTuples(*childTuplesInput, *childTuples);
#ifdef VERBOSE
				printBagContents(node->getVertices());
				std::cout << "Child tuple input:" << std::endl << childTuplesInput->str() << std::endl;
#endif

				Streams inputStreams;
				inputStreams.addFile(aspProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
				// Remember: "Streams" deletes the appended streams -_-
				inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
				inputStreams.appendStream(Streams::StreamPtr(childTuplesInput), "<child_tuples>");

				GringoOutputProcessor outputProcessor;
				ClaspInputReader inputReader(inputStreams, outputProcessor);
				ModelProcessor cb(*this, *newTuples, outputProcessor);
				Clasp::ClaspConfig config;
				config.enumerate.numModels = 0;
				config.master()->heuristic().name = "none";
				// TODO: Projection
				clasp.solve(inputReader, config, &cb);
			} else {
				assert(algorithmType == SEMI);
				// XXX: Too many auxiliary sets...
				VertexSet removedAtoms, removedRules;
				foreach(Vertex v, node->getRemovedVertices()) {
					if(problem.vertexIsRule(v))
						removedRules.insert(v);
					else
						removedAtoms.insert(v);
				}
				VertexSet introducedAtoms, introducedRules;
				foreach(Vertex v, node->getIntroducedVertices()) {
					if(problem.vertexIsRule(v))
						introducedRules.insert(v);
					else
						introducedAtoms.insert(v);
				}
				VertexSet currentAtoms;
				std::map<Vertex,VertexSet> enforcedIfPos, enforcedIfNeg; // enforcedIfPos[a] is the set of rules that are made true by the atom a being true
				foreach(Vertex v, node->getVertices()) {
					if(problem.vertexIsRule(v)) {
						const Problem::VerticesInRule& atoms = problem.getAtomsInRule(v);
						foreach(Vertex a, atoms.head)
							enforcedIfPos[a].insert(v);
						foreach(Vertex a, atoms.pos)
							enforcedIfNeg[a].insert(v);
						foreach(Vertex a, atoms.neg)
							enforcedIfPos[a].insert(v);
					}
					else
						currentAtoms.insert(v);
				}
				VertexSet commonAtoms;
				std::set_difference(currentAtoms.begin(), currentAtoms.end(), introducedAtoms.begin(), introducedAtoms.end(), std::inserter(commonAtoms, commonAtoms.end()));

				for(TupleSet::const_iterator cit = childTuples->begin(); cit != childTuples->end(); ++cit) {
					Tuple& childTuple = *dynamic_cast<Tuple*>(cit->first);
					// If there is a removed rule which this child tuple does not contain, skip the tuple
					if(!std::includes(childTuple.rules.begin(), childTuple.rules.end(), removedRules.begin(), removedRules.end()))
						continue;

					// Which introduced rules are already satisfied by the child tuple?
					VertexSet satisfiedIntroducedRules;
					VertexSet falseAtoms; // does not include introduced atoms
					std::set_difference(commonAtoms.begin(), commonAtoms.end(), childTuple.atoms.begin(), childTuple.atoms.end(), std::inserter(falseAtoms, falseAtoms.end()));

					// For introduced atoms, generate all subsets
					PowerSet<VertexSet> subsets(introducedAtoms);
					for(PowerSet<VertexSet>::const_iterator sit = subsets.begin(); sit != subsets.end(); ++sit) {
						const std::pair<VertexSet,VertexSet>& posAndNeg = *sit;

						Tuple& t = *new Tuple;
						// Inherit non-removed atoms and rules
						std::set_difference(childTuple.atoms.begin(), childTuple.atoms.end(), removedAtoms.begin(), removedAtoms.end(), std::inserter(t.atoms, t.atoms.end()));
						std::set_difference(childTuple.rules.begin(), childTuple.rules.end(), removedRules.begin(), removedRules.end(), std::inserter(t.rules, t.rules.end()));

						// Rule introduction
						foreach(Vertex v, t.atoms)
							t.rules.insert(enforcedIfPos[v].begin(), enforcedIfPos[v].end());
						foreach(Vertex v, falseAtoms)
							t.rules.insert(enforcedIfNeg[v].begin(), enforcedIfNeg[v].end());

						// Insert atoms and enforced rules
						t.atoms.insert(posAndNeg.first.begin(), posAndNeg.first.end());
						foreach(Vertex v, posAndNeg.first)
							t.rules.insert(enforcedIfPos[v].begin(), enforcedIfPos[v].end());
						foreach(Vertex v, posAndNeg.second)
							t.rules.insert(enforcedIfNeg[v].begin(), enforcedIfNeg[v].end());

						// Add tuple and solution
						Solution* s = addToSolution(cit->second, t.atoms);
						addToTupleSet(&t, s, newTuples);
					}
				}
			}
		}

		delete childTuples;
	}
	else { // Leaf
#if PROGRESS_REPORT > 0
		printProgressLine(node);
#if PROGRESS_REPORT > 2
		boost::timer::auto_cpu_timer timer(" %ts\n");
#endif
#endif
#ifdef VERBOSE
		printBagContents(node->getVertices());
#endif
		if(algorithmType == SEMI_ASP) {
			Streams inputStreams;
			inputStreams.addFile(aspProgram, false);
			inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");

			GringoOutputProcessor outputProcessor;
			ClaspInputReader inputReader(inputStreams, outputProcessor);
			ModelProcessor cb(*this, *newTuples, outputProcessor);
			Clasp::ClaspConfig config;
			config.enumerate.numModels = 0;
			config.master()->heuristic().name = "none";
			clasp.solve(inputReader, config, &cb);
		} else {
			assert(algorithmType == SEMI);
			VertexSet currentAtoms;
			std::map<Vertex,VertexSet> enforcedIfPos, enforcedIfNeg; // enforcedIfPos[a] is the set of rules that are made true by the atom a being true
			foreach(Vertex v, node->getVertices()) {
				if(problem.vertexIsRule(v)) {
					const Problem::VerticesInRule& atoms = problem.getAtomsInRule(v);
					foreach(Vertex a, atoms.head)
						enforcedIfPos[a].insert(v);
					foreach(Vertex a, atoms.pos)
						enforcedIfNeg[a].insert(v);
					foreach(Vertex a, atoms.neg)
						enforcedIfPos[a].insert(v);
				} else
					currentAtoms.insert(v);
			}

			PowerSet<VertexSet> subsets(currentAtoms);
			// Generate all subsets of the current atoms and add rules enforced by the respective subset
			for(PowerSet<VertexSet>::const_iterator it = subsets.begin(); it != subsets.end(); ++it) {
				const std::pair<VertexSet,VertexSet>& posAndNeg = *it;
				Tuple& t = *new Tuple;
				t.atoms = posAndNeg.first;
				foreach(Vertex v, posAndNeg.first)
					t.rules.insert(enforcedIfPos[v].begin(), enforcedIfPos[v].end());
				foreach(Vertex v, posAndNeg.second)
					t.rules.insert(enforcedIfNeg[v].begin(), enforcedIfNeg[v].end());

				Solution* s = createLeafSolution(t.atoms);
				addToTupleSet(&t, s, newTuples);
			}
		}
	}

#ifdef VERBOSE
	std::cout << "Resulting tuples:" << std::endl;
	for(TupleSet::const_iterator it = newTuples->begin(); it != newTuples->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout, problem);
	std::cout << std::endl;
#endif

	return newTuples;
}

#if PROGRESS_REPORT > 0
sharp::TupleSet* Algorithm::evaluateNode(const sharp::ExtendedHypertree* node) {
	sharp::TupleSet* ts = sharp::AbstractSemiNormalizedHTDAlgorithm::evaluateNode(node);
	++nodesProcessed;
	return ts;
}

void Algorithm::printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples) {
#if PROGRESS_REPORT == 1
	std::cout << '\r' << "Processing node ";
	std::cout << std::setw(4) << std::left << (nodesProcessed+1) << " [";
#endif
	switch(node->getType()) {
		case sharp::Leaf:
			std::cout << 'L';
			break;
		case sharp::Permutation:
			std::cout << 'E';
			break;
		case sharp::Branch:
			std::cout << 'J';
			break;
		case sharp::Introduction:
			assert(typeid(*this) != typeid(Algorithm)); // This cannot happen with the semi-normalized algorithm, but only with the normalized one
			std::cout << 'I';
			break;
		case sharp::Removal:
			assert(typeid(*this) != typeid(Algorithm));
			std::cout << 'R';
			break;
		default:
			assert(false);
			std::cout << '?';
			break;
	}
#if PROGRESS_REPORT == 1
	std::cout << "] " << std::setw(2) << std::right << node->getVertices().size() << " elements ["
		<< std::setw(2) << node->getRemovedVertices().size() << "R"
		<< std::setw(2) << node->getIntroducedVertices().size() << "I] "
		<< std::setw(7) << numChildTuples << " child tuples"
		<< std::flush;
#elif PROGRESS_REPORT > 1
	std::cout << std::setw(4) << std::right << (nodesProcessed+1) << ": ";

	size_t numCurrentRules = 0;
	size_t numCurrentAtoms = 0;
	foreach(sharp::Vertex v, node->getVertices()) {
		if(problem.vertexIsRule(v))
			++numCurrentRules;
		else
			++numCurrentAtoms;
	}
	size_t numRemovedRules = 0;
	size_t numRemovedAtoms = 0;
	foreach(sharp::Vertex v, node->getRemovedVertices()) {
		if(problem.vertexIsRule(v))
			++numRemovedRules;
		else
			++numRemovedAtoms;
	}
	size_t numIntroducedRules = 0;
	size_t numIntroducedAtoms = 0;
	foreach(sharp::Vertex v, node->getIntroducedVertices()) {
		if(problem.vertexIsRule(v))
			++numIntroducedRules;
		else
			++numIntroducedAtoms;
	}
	size_t numCommonRules = numCurrentRules - numIntroducedRules;

	std::cout << std::setw(2) << numCurrentAtoms << " A " << std::setw(2) << numCurrentRules << " R -> " << std::setw(2) << node->getVertices().size();

	std::cout << ";  Int " << std::setw(2) << numIntroducedAtoms << " A " << std::setw(2) << numIntroducedRules << " R";
	std::cout << ";  Rem " << std::setw(2) << numRemovedAtoms << " A " << std::setw(2) << numRemovedRules << " R";

	std::cout << ";  Com " << std::setw(2) << numCommonRules << " R";
	
	std::cout << ";  Child tuples " << std::setw(7) << numChildTuples;
#if PROGRESS_REPORT < 3
	std::cout << std::endl;
#else
	std::cout << ' ' << std::flush;
#endif
#endif
}
#endif

#ifdef VERBOSE
void Algorithm::printBagContents(const sharp::VertexSet& vertices) const
{
	std::cout << "Bag contents:" << std::endl;
	foreach(Vertex v, vertices) {
		if(problem.vertexIsRule(v))
			std::cout << "rule " << v << std::endl;
		else
			std::cout << "atom " << problem.getVertexName(v) << '[' << v << "]" << std::endl;
	}
}
#endif
