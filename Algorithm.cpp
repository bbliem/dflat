#include <iostream>
#include <iomanip>
#include <gringo/streams.h>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Algorithm.h"
#include "ClaspInputReader.h"
#include "Problem.h"
#include "Tuple.h"
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

			foreach(Vertex v, tuple.m)
				ss << "oldM(m" << mi << ",v" << v << ")." << std::endl;
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


	// FIXME: I'm ugly.
	inline bool equalAtoms(const VertexSet& l, const VertexSet& r, Problem& problem) {
		// Supposes that the traversal order of the atoms is the same
		VertexSet::const_iterator lit = l.begin();
		VertexSet::const_iterator rit = r.begin();

		for(;;) {
			while(lit != l.end() && problem.vertexIsRule(*lit))
				++lit;
			while(rit != r.end() && problem.vertexIsRule(*rit))
				++rit;

			if(lit == l.end() || rit == r.end())
				break;

			if(*lit != *rit)
				return false;

			++lit;
			++rit;
		}

		return lit == l.end() && rit == r.end();
	}
}

Algorithm::Algorithm(sharp::Problem& problem, Algorithm::ProblemType problemType)
	: AbstractSemiNormalizedHTDAlgorithm(&problem), problemType(problemType)
#ifndef NO_PROGRESS_REPORT
	  , nodesProcessed(0)
#endif
{
}

Algorithm::~Algorithm()
{
}

Solution* Algorithm::selectSolution(TupleSet* tuples, const ExtendedHypertree* root)
{
#ifndef NO_PROGRESS_REPORT
	std::cout << std::endl; // End progress line
#endif

	Solution* result = createEmptySolution();

	VertexSet currentRules;
	foreach(Vertex v, root->getVertices())
		if(dynamic_cast<Problem*>(problem())->vertexIsRule(v))
			currentRules.insert(v);

	for(TupleSet::iterator it = tuples->begin(); it != tuples->end(); ++it) {
		const Tuple& t = *dynamic_cast<Tuple*>(it->first);

		// A tuple corresponds to a valid solution if all rules in the current bag are in M
		if(std::includes(t.m.begin(), t.m.end(), currentRules.begin(), currentRules.end()))
			result = combineSolutions(sharp::Union, result, it->second);
	}

	return result;
}

TupleSet* Algorithm::evaluateBranchNode(const ExtendedHypertree* node)
{
	TupleSet* left = evaluateNode(node->firstChild());
	TupleSet* right = evaluateNode(node->secondChild());
	printProgressLine(node);
	TupleSet* ts = new TupleSet;

	for(TupleSet::const_iterator lit = left->begin(); lit != left->end(); ++lit) {
		Tuple &l = *(Tuple *)lit->first;
		for(TupleSet::const_iterator rit = right->begin(); rit != right->end(); ++rit) {
			Tuple &r = *(Tuple *)rit->first;
			if(!equalAtoms(l.m, r.m, *dynamic_cast<Problem*>(problem()))) continue;

			Tuple& ast = *new Tuple;

			ast.m = l.m;
			ast.m.insert(r.m.begin(), r.m.end());

			Solution *s = combineSolutions(sharp::CrossJoin, lit->second, rit->second);

			// FIXME: It seems this is the same as Algorithm::addToTupleSet. Use that instead?
			std::pair<TupleSet::iterator, bool> result = ts->insert(TupleSet::value_type(&ast, s));
			if(!result.second) {
				Solution *orig = result.first->second;
				ts->erase(result.first);
				ts->insert(TupleSet::value_type(&ast, combineSolutions(sharp::Union, orig, s)));
			}
		}
	}

	delete left;
	delete right;

#ifdef VERBOSE
	std::cout << "Join node result:" << std::endl;
	for(TupleSet::const_iterator it = ts->begin(); it != ts->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout, *dynamic_cast<Problem*>(problem()));
	std::cout << std::endl;
#endif

	return ts;
}

TupleSet* Algorithm::evaluatePermutationNode(const ExtendedHypertree* node)
{
	// TODO: Make configurable
	const char* program = problemType == DECISION ? "exchange_decision.lp" : "exchange.lp";

	TupleSet* newTuples = new TupleSet;

	// Build input for the exchange node program
	std::stringstream* bagContents = new std::stringstream;
	describeExchangeNodeContents(*bagContents, *node, *dynamic_cast<Problem*>(problem()));

	if(node->getType() != sharp::Leaf) {
		TupleSet* childTuples = evaluateNode(node->firstChild());
		printProgressLine(node);
		// There might be no child tuples, consider as a child e.g. a join node without matches.
		// If we were to run the program without child tuples, it would consider the current node as a leaf node and wrongly generate new tuples.
		if(childTuples->empty() == false) {
			std::stringstream* childTuplesInput = new std::stringstream;
			describeChildTuples(*childTuplesInput, *childTuples);
#ifdef VERBOSE
			std::cout << "Exchange node; bag contents:" << std::endl;
			std::cout << bagContents->str() << std::endl;
			foreach(Vertex v, node->getVertices()) {
				if(dynamic_cast<Problem*>(problem())->vertexIsRule(v))
					std::cout << 'r' << v << std::endl;
				else
					std::cout << problem()->getVertexName(v) << " (v" << v << ")" << std::endl;
			}
			std::cout << "Child tuple input:" << std::endl << childTuplesInput->str() << std::endl;
#endif

			Streams inputStreams;
			inputStreams.addFile(program, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
			// Remember: "Streams" deletes the appended streams -_-
			inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
			inputStreams.appendStream(Streams::StreamPtr(childTuplesInput), "<child_tuples>");

			GringoOutputProcessor outputProcessor;
			ClaspInputReader inputReader(inputStreams, outputProcessor);
			ModelProcessor cb(*this, *newTuples, outputProcessor);
			Clasp::ClaspConfig config;
			config.enumerate.numModels = 0;
			// TODO: Projection
			clasp.solve(inputReader, config, &cb);
		}

		delete childTuples;
	}
	else {
		// This is a leaf, run the program once
		printProgressLine(node);
#ifdef VERBOSE
		std::cout << "Leaf node; bag contents:" << std::endl;
		std::cout << bagContents->str() << std::endl;
		foreach(Vertex v, node->getVertices()) {
			if(dynamic_cast<Problem*>(problem())->vertexIsRule(v))
				std::cout << 'r' << v << std::endl;
			else
				std::cout << problem()->getVertexName(v) << " (v" << v << ")" << std::endl;
		}
#endif

		Streams inputStreams;
		inputStreams.addFile(program, false);
		inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");

		GringoOutputProcessor outputProcessor;
		ClaspInputReader inputReader(inputStreams, outputProcessor);
		ModelProcessor cb(*this, *newTuples, outputProcessor);
		Clasp::ClaspConfig config;
		config.enumerate.numModels = 0;
		clasp.solve(inputReader, config, &cb);
	}

#ifdef VERBOSE
	std::cout << "Resulting tuples:" << std::endl;
	for(TupleSet::const_iterator it = newTuples->begin(); it != newTuples->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout, *dynamic_cast<Problem*>(problem()));
	std::cout << std::endl;
#endif

	return newTuples;
}

#ifndef NO_PROGRESS_REPORT
sharp::TupleSet* Algorithm::evaluateNode(const sharp::ExtendedHypertree* node) {
	sharp::TupleSet* ts = sharp::AbstractSemiNormalizedHTDAlgorithm::evaluateNode(node);
	++nodesProcessed;
	return ts;
}

inline void Algorithm::printProgressLine(const sharp::ExtendedHypertree* node) {
	std::cout << '\r' << "Processing node ";
	std::cout << std::setw(4) << std::left << (nodesProcessed+1) << " [";
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
		default:
			assert(false);
			std::cout << '?';
			break;
	}
	std::cout << "] bag size " << std::setw(3) << node->getVertices().size() << std::flush;
}
#endif
