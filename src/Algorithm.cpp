#include <iostream>
#include <iomanip>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#ifdef WITH_NODE_TIMER
#include <boost/timer/timer.hpp>
#endif

#include "Algorithm.h"
#include "Tuple.h"

using sharp::Solution;
using sharp::TupleSet;
using sharp::ExtendedHypertree;
using sharp::VertexSet;
using sharp::Vertex;

Algorithm::Algorithm(sharp::Problem& problem, sharp::NormalizationType normalizationType)
	: AbstractSemiNormalizedHTDAlgorithm(&problem), problem(problem), normalizationType(normalizationType)
#if PROGRESS_REPORT > 0
	  , nodesProcessed(0)
#endif
{
	assert(normalizationType == sharp::DefaultNormalization || normalizationType == sharp::SemiNormalization);
}

Solution* Algorithm::selectSolution(TupleSet* tuples, const ExtendedHypertree* root)
{
#if PROGRESS_REPORT > 0
	std::cout << '\r' << std::setw(66) << std::left << "Done." << std::endl; // Clear/end progress line
#endif

	Solution* result = createEmptySolution();

	for(TupleSet::iterator it = tuples->begin(); it != tuples->end(); ++it)
		result = combineSolutions(sharp::Union, result, it->second);

	return result;
}

TupleSet* Algorithm::evaluateBranchNode(const ExtendedHypertree* node)
{
	TupleSet* left = evaluateNode(node->firstChild());
	TupleSet* right = evaluateNode(node->secondChild());
#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif
#if PROGRESS_REPORT > 0
	printProgressLine(node, left->size()+right->size());
#endif
#ifdef WITH_NODE_TIMER
	boost::timer::auto_cpu_timer timer(" %ts\n");
#endif
	assert(node->getIntroducedVertices().empty() && node->getRemovedVertices().empty());
	TupleSet* newTuples = join(node->getVertices(), *left, *right);

	delete left;
	delete right;

#ifdef VERBOSE
	std::cout << std::endl << "Resulting tuples of join node:" << std::endl;
	for(TupleSet::const_iterator it = newTuples->begin(); it != newTuples->end(); ++it)
		dynamic_cast<const Tuple*>(it->first)->print(std::cout, problem);
	std::cout << std::endl;
#endif

	return newTuples;
}

sharp::TupleSet* Algorithm::evaluatePermutationNode(const sharp::ExtendedHypertree* node)
{
	TupleSet* childTuples = 0;

	if(node->getType() != sharp::Leaf)
		childTuples = evaluateNode(node->firstChild());

#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif
#if PROGRESS_REPORT > 0
	printProgressLine(node, childTuples ? childTuples->size() : 0);
#endif
#ifdef WITH_NODE_TIMER
	boost::timer::auto_cpu_timer timer(" %ts\n");
#endif

	TupleSet* newTuples;

	if(childTuples) {
		assert(node->getType() != sharp::Leaf);
		newTuples = exchangeNonLeaf(node->getVertices(), node->getIntroducedVertices(), node->getRemovedVertices(), *childTuples);
		delete childTuples;
	} else {
		assert(node->getType() == sharp::Leaf);
//		newTuples = exchangeLeaf(node->getVertices(), node->getIntroducedVertices(), node->getRemovedVertices());
		newTuples = exchangeLeaf(node->getVertices(), node->getVertices(), node->getRemovedVertices());
	}

#ifdef VERBOSE
	std::cout << std::endl << "Resulting tuples of exchange node:" << std::endl;
	for(TupleSet::const_iterator it = newTuples->begin(); it != newTuples->end(); ++it)
		dynamic_cast<Tuple*>(it->first)->print(std::cout, problem);
	std::cout << std::endl;
#endif

	return newTuples;
}

sharp::TupleSet* Algorithm::join(const sharp::VertexSet&, sharp::TupleSet& left, sharp::TupleSet& right)
{
	TupleSet* ts = new TupleSet;

	// TupleSets are ordered, use sort merge join algorithm
	TupleSet::const_iterator lit = left.begin();
	TupleSet::const_iterator rit = right.begin();
#define TUP(X) (*dynamic_cast<const Tuple*>(X->first)) // FIXME: Think of something better
	while(lit != left.end() && rit != right.end()) {
		while(!TUP(lit).matches(TUP(rit))) {
			// Advance iterator pointing to smaller value
			if(TUP(lit) < TUP(rit)) {
				++lit;
				if(lit == left.end())
					goto endJoin;
			} else {
				++rit;
				if(rit == right.end())
					goto endJoin;
			}
		}

		// Now lit and rit join
		// Remember position of rit and advance rit until no more match
		TupleSet::const_iterator mark = rit;
joinLitWithAllPartners:
		do {
			sharp::Tuple* t = TUP(lit).join(TUP(rit));
			Solution *s = combineSolutions(sharp::CrossJoin, lit->second, rit->second);
			// FIXME: It seems this is the same as Algorithm::addToTupleSet. Use that instead?
			std::pair<TupleSet::iterator, bool> result = ts->insert(TupleSet::value_type(t, s));
			if(!result.second) {
				Solution *orig = result.first->second;
				ts->erase(result.first);
				ts->insert(TupleSet::value_type(t, combineSolutions(sharp::Union, orig, s)));
			}
			++rit;
		} while(rit != right.end() && TUP(lit).matches(TUP(rit)));

		// lit and rit don't join anymore. Advance lit. If it joins with mark, reset rit to mark.
		++lit;
		if(lit == left.end())
			break;

		if(TUP(lit).matches(TUP(mark))) {
			rit = mark;
			goto joinLitWithAllPartners; // Ha!
		}
	}
endJoin:
	return ts;
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

#if PROGRESS_REPORT > 0
sharp::TupleSet* Algorithm::evaluateNode(const sharp::ExtendedHypertree* node) {
	sharp::TupleSet* ts = sharp::AbstractSemiNormalizedHTDAlgorithm::evaluateNode(node);
	++nodesProcessed;
	return ts;
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
