#include "ClaspAlgorithm.h"
#include "Problem.h"
#include "GringoOutputProcessor.h"
#include "ClaspCallback.h"
#if PROGRESS_REPORT > 1
#include <iomanip>
#endif

namespace sat {

ClaspAlgorithm::ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram)
	: ::ClaspAlgorithm(problem, exchangeNodeProgram)
{
}


std::auto_ptr<Clasp::ClaspFacade::Callback> ClaspAlgorithm::newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor& gringoOutput) const
{
	return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallback(*this, newTuples, dynamic_cast<const GringoOutputProcessor&>(gringoOutput)));
}

std::auto_ptr< ::GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr< ::GringoOutputProcessor>(new GringoOutputProcessor);
}

#if PROGRESS_REPORT > 1
void ClaspAlgorithm::printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples) {
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
	std::cout << std::setw(4) << std::right << (nodesProcessed+1) << ": ";

	size_t numCurrentRules = 0;
	size_t numCurrentAtoms = 0;
	foreach(sharp::Vertex v, node->getVertices()) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
			++numCurrentRules;
		else
			++numCurrentAtoms;
	}
	size_t numRemovedRules = 0;
	size_t numRemovedAtoms = 0;
	foreach(sharp::Vertex v, node->getRemovedVertices()) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
			++numRemovedRules;
		else
			++numRemovedAtoms;
	}
	size_t numIntroducedRules = 0;
	size_t numIntroducedAtoms = 0;
	foreach(sharp::Vertex v, node->getIntroducedVertices()) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
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
#ifndef WITH_NODE_TIMER
	std::cout << std::endl;
#else
	std::cout << ' ' << std::flush;
#endif
}
#endif

#ifdef VERBOSE
void ClaspAlgorithm::printBagContents(const sharp::VertexSet& vertices) const
{
	std::cout << "Bag contents:" << std::endl;
	foreach(sharp::Vertex v, vertices) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
			std::cout << "rule " << v << std::endl;
		else
			std::cout << "atom " << problem.getVertexName(v) << '[' << v << "]" << std::endl;
	}
}
#endif

} // namespace sat
