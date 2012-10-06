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
#include <gringo/streams.h>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#ifdef WITH_NODE_TIMER
#include <boost/timer/timer.hpp>
#endif

#include "Algorithm.h"
#include "Tuple.h"
#include "ClaspInputReader.h"
#include "ClaspCallbackGeneral.h"
#include "ClaspCallbackNP.h"

using sharp::Plan;
using sharp::PlanFactory;
using sharp::TupleTable;
using sharp::ExtendedHypertree;
using sharp::VertexSet;
using sharp::Vertex;

namespace {
	inline void declareBagContents(std::ostream& bagContents, const sharp::Problem& problem, const std::string& instanceFacts, const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
	{
		bagContents << instanceFacts << std::endl;

		foreach(sharp::Vertex v, vertices)
			bagContents << "current(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
		foreach(sharp::Vertex v, introduced)
			bagContents << "introduced(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
		foreach(sharp::Vertex v, removed)
			bagContents << "removed(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
	}
}

Algorithm::Algorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram, sharp::NormalizationType normalizationType, unsigned int level)
	: AbstractSemiNormalizedHTDAlgorithm(&problem, planFactory), problem(problem), normalizationType(normalizationType), instanceFacts(instanceFacts), exchangeNodeProgram(exchangeNodeProgram), joinNodeProgram(joinNodeProgram), level(level)
#ifdef PROGRESS_REPORT
	  , nodesProcessed(0)
#endif
{
	assert(normalizationType == sharp::DefaultNormalization || normalizationType == sharp::SemiNormalization);
}

Plan* Algorithm::selectPlan(TupleTable* table, const ExtendedHypertree* root)
{
#ifdef PROGRESS_REPORT
	std::cout << '\r' << std::setw(66) << std::left << "Done." << std::endl; // Clear/end progress line
#endif

	if(table->empty())
		return 0;

	TupleTable::const_iterator it = table->begin();
	Plan* result = it->second;

	for(++it; it != table->end(); ++it)
		result = planFactory.unify(result, it->second);

	return result;
}

sharp::ExtendedHypertree* Algorithm::prepareHypertreeDecomposition(sharp::ExtendedHypertree* root)
{
	assert(normalizationType == sharp::DefaultNormalization || normalizationType == sharp::SemiNormalization);
	//return root->normalize(normalizationType);

	// FIXME: This is a workaround until SHARP lets us insert an empty root in the normalization
	sharp::ExtendedHypertree* newRoot = new sharp::ExtendedHypertree(sharp::VertexSet());
	newRoot->insChild(root);

	return newRoot->normalize(normalizationType);
}


sharp::TupleTable* Algorithm::evaluatePermutationNode(const sharp::ExtendedHypertree* node)
{
	TupleTable* childTable = 0;

	if(node->getType() != sharp::Leaf)
		childTable = evaluateNode(node->firstChild());

#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif
#ifdef PROGRESS_REPORT
	printProgressLine(node, childTable ? childTable->size() : 0);
#endif
#ifdef WITH_NODE_TIMER
	boost::timer::auto_cpu_timer timer(" %ts\n");
#endif

	TupleTable* newTable;

	if(childTable) {
		assert(node->getType() != sharp::Leaf);
		newTable = exchangeNonLeaf(node->getVertices(), node->getIntroducedVertices(), node->getRemovedVertices(), *childTable);
		delete childTable;
	} else {
		assert(node->getType() == sharp::Leaf);
		newTable = exchangeLeaf(node->getVertices(), node->getVertices(), node->getRemovedVertices());
	}

#ifdef VERBOSE
	std::cout << std::endl << "Resulting tuples of exchange node:" << std::endl;
	for(TupleTable::const_iterator it = newTable->begin(); it != newTable->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout);
	std::cout << std::endl;
#endif

	return newTable;
}

TupleTable* Algorithm::evaluateBranchNode(const ExtendedHypertree* node)
{
	TupleTable* left = evaluateNode(node->firstChild());
	TupleTable* right = evaluateNode(node->secondChild());
#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif
#ifdef PROGRESS_REPORT
	printProgressLine(node, left->size()+right->size());
#endif
#ifdef WITH_NODE_TIMER
	boost::timer::auto_cpu_timer timer(" %ts\n");
#endif
	assert(node->getIntroducedVertices().empty() && node->getRemovedVertices().empty());
	TupleTable* newTable = join(node->getVertices(), *left, *right);

	delete left;
	delete right;

#ifdef VERBOSE
	std::cout << std::endl << "Resulting tuples of join node:" << std::endl;
	for(TupleTable::const_iterator it = newTable->begin(); it != newTable->end(); ++it)
		dynamic_cast<const Tuple*>(it->first)->print(std::cout);
	std::cout << std::endl;
#endif

	return newTable;
}

TupleTable* Algorithm::exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
{
	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, problem, instanceFacts, vertices, introduced, removed);

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);

	TupleTable* newTable = new TupleTable;
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor, vertices);
	Clasp::ClaspConfig config;
	config.enumerate.numModels = 0;
	clasp.solve(inputReader, config, cb.get());

	return newTable;
}

TupleTable* Algorithm::exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleTable& childTable)
{
	TupleTable* newTable = new TupleTable;
	// There might be no child tuples, consider as a child e.g. a join node without matches.
	// If we were to run the program without child tuples, it would consider the current node as a leaf node and wrongly generate new tuples.
	if(childTable.empty())
		return newTable;

	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, problem, instanceFacts, vertices, introduced, removed);

	std::stringstream* childTableInput = new std::stringstream;
	// Declare child tuples
	foreach(const TupleTable::value_type& tupleAndSolution, childTable)
		dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution);
//#ifdef VERBOSE
//	std::cout << std::endl << "Child tuple input:" << std::endl << childTableInput->str() << std::endl;
//#endif

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
	inputStreams.appendStream(Streams::StreamPtr(childTableInput), "<child_tuples>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor, vertices);
	Clasp::ClaspConfig config;
	config.enumerate.numModels = 0;
	clasp.solve(inputReader, config, cb.get());

	return newTable;
}

TupleTable* Algorithm::join(const sharp::VertexSet& vertices, sharp::TupleTable& childTableLeft, sharp::TupleTable& childTableRight)
{
	TupleTable* newTable = new TupleTable;

	if(joinNodeProgram) {
		std::stringstream* bagContents = new std::stringstream;
		declareBagContents(*bagContents, problem, instanceFacts, vertices, sharp::VertexSet(), sharp::VertexSet());

		std::stringstream* childTableInput = new std::stringstream;
		// Declare child tuples
		foreach(const TupleTable::value_type& tupleAndSolution, childTableLeft)
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution, "L");
		foreach(const TupleTable::value_type& tupleAndSolution, childTableRight)
			dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution, "R");
		//#ifdef VERBOSE
		//	std::cout << std::endl << "Child tuple input:" << std::endl << childTableInput->str() << std::endl;
		//#endif

		Streams inputStreams;
		inputStreams.addFile(joinNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
		// Remember: "Streams" deletes the appended streams -_-
		inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
		inputStreams.appendStream(Streams::StreamPtr(childTableInput), "<child_tuples>");

		std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
		ClaspInputReader inputReader(inputStreams, *outputProcessor);

		std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor, vertices);
		Clasp::ClaspConfig config;
		config.enumerate.numModels = 0;
		clasp.solve(inputReader, config, cb.get());
	}

	else {
		// Default join implementation (used when no join node program is specified)
		// TupleTables are ordered, use sort merge join algorithm
		TupleTable::const_iterator lit = childTableLeft.begin();
		TupleTable::const_iterator rit = childTableRight.begin();
#define TUP(X) (*dynamic_cast<const Tuple*>(X->first)) // FIXME: Think of something better
		while(lit != childTableLeft.end() && rit != childTableRight.end()) {
			while(!TUP(lit).matches(TUP(rit))) {
				// Advance iterator pointing to smaller value
				if(TUP(lit) < TUP(rit)) {
					++lit;
					if(lit == childTableLeft.end())
						goto endJoin;
				} else {
					++rit;
					if(rit == childTableRight.end())
						goto endJoin;
				}
			}

			// Now lit and rit join
			// Remember position of rit and advance rit until no more match
			TupleTable::const_iterator mark = rit;
joinLitWithAllPartners:
			do {
				sharp::Tuple* t = TUP(lit).join(TUP(rit));
				Plan* p = planFactory.join(lit->second, rit->second, *t);
				addRowToTupleTable(*newTable, t, p);
				++rit;
			} while(rit != childTableRight.end() && TUP(lit).matches(TUP(rit)));

			// lit and rit don't join anymore. Advance lit. If it joins with mark, reset rit to mark.
			++lit;
			if(lit == childTableLeft.end())
				break;

			if(TUP(lit).matches(TUP(mark))) {
				rit = mark;
				goto joinLitWithAllPartners; // Ha!
			}
		}
endJoin:
		;
	}

	return newTable;
}

std::auto_ptr<Clasp::ClaspFacade::Callback> Algorithm::newClaspCallback(sharp::TupleTable& newTable, const GringoOutputProcessor& gringoOutput, const sharp::VertexSet& currentVertices) const
{
#ifndef NDEBUG
	std::set<std::string> currentVertexNames;
	foreach(sharp::Vertex v, currentVertices)
		currentVertexNames.insert(const_cast<sharp::Problem&>(problem).getVertexName(v));

	if(level == 0)
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackNP(*this, newTable, gringoOutput, currentVertexNames));
	else
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackGeneral(*this, newTable, gringoOutput, level, currentVertexNames));
#else
	if(level == 0)
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackNP(*this, newTable, gringoOutput));
	else
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackGeneral(*this, newTable, gringoOutput, level));
#endif
}

std::auto_ptr<GringoOutputProcessor> Algorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr<GringoOutputProcessor>(new GringoOutputProcessor());
}

#ifdef PROGRESS_REPORT
sharp::TupleTable* Algorithm::evaluateNode(const sharp::ExtendedHypertree* node) {
	sharp::TupleTable* tt = sharp::AbstractSemiNormalizedHTDAlgorithm::evaluateNode(node);
	++nodesProcessed;
	return tt;
}

void Algorithm::printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples) {
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
		case sharp::Introduction:
			std::cout << 'I';
			break;
		case sharp::Removal:
			std::cout << 'R';
			break;
		default:
			assert(false);
			std::cout << '?';
			break;
	}
	std::cout << "] " << std::setw(2) << std::right << node->getVertices().size() << " elements ["
		<< std::setw(2) << node->getRemovedVertices().size() << "R"
		<< std::setw(2) << node->getIntroducedVertices().size() << "I] "
		<< std::setw(7) << numChildTuples << " child tuples"
		<< std::flush;
}
#endif

#ifdef VERBOSE
void Algorithm::printBagContents(const sharp::VertexSet& vertices) const
{
	std::cout << "Bag contents: ";
	foreach(Vertex v, vertices)
		std::cout << problem.getVertexName(v) << ' ';
	std::cout << std::endl;
}
#endif
