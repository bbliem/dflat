#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "AlgorithmNormalized.h"
#include "Problem.h"
#include "Tuple.h"

using sharp::TupleSet;
using sharp::ExtendedHypertree;

AlgorithmNormalized::AlgorithmNormalized(Problem& problem, AlgorithmNormalized::ProblemType problemType, AlgorithmType algorithmType)
	: Algorithm(problem, problemType, algorithmType)
{
}

AlgorithmNormalized::~AlgorithmNormalized()
{
}

ExtendedHypertree* AlgorithmNormalized::prepareHypertreeDecomposition(ExtendedHypertree* root)
{
	return root->normalize(sharp::DefaultNormalization);
}

TupleSet* AlgorithmNormalized::evaluatePermutationNode(const ExtendedHypertree* node)
{
	switch(algorithmType) {
		case SEMI: case SEMI_ASP:
			return Algorithm::evaluatePermutationNode(node);
		case NORMALIZED:
			switch(node->getType()) {
				case sharp::Introduction:
					return evaluateIntroductionNode(node);
				case sharp::Removal:
					return evaluateRemovalNode(node);
				case sharp::Leaf:
					// XXX: Using permutation node ASP program of the semi-normalized Algorithm. Maybe implement it in C++?
					return Algorithm::evaluatePermutationNode(node);
				default:
					assert(false);
					return 0;
			}
	}
	assert(false);
	return 0;
}

TupleSet* AlgorithmNormalized::evaluateIntroductionNode(const ExtendedHypertree* node)
{
	TupleSet* childTuples = evaluateNode(node->firstChild());
#if PROGRESS_REPORT> 0
	printProgressLine(node, childTuples->size());
#endif
#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif

	TupleSet* newTuples = new TupleSet;
	sharp::Vertex introduced = node->getDifference();

	if(problem.vertexIsRule(introduced)) {
		const Problem::VerticesInRule& ruleAtoms = problem.getAtomsInRule(introduced);

		for(TupleSet::const_iterator it = childTuples->begin(); it != childTuples->end(); ++it) {
			Tuple& t = dynamic_cast<Tuple&>(*it->first);

			sharp::VertexSet trueAtoms;
			sharp::VertexSet falseAtoms;
			foreach(sharp::Vertex v, node->getVertices()) {
				if(t.atoms.find(v) == t.atoms.end())
					falseAtoms.insert(v);
				else
					trueAtoms.insert(v);
			}

			Tuple* newTuple = new Tuple(t);

			// XXX: Does find_first_of take advantage of the sortedness?
			if(std::find_first_of(ruleAtoms.head.begin(), ruleAtoms.head.end(), trueAtoms.begin(), trueAtoms.end()) != ruleAtoms.head.end()
					|| std::find_first_of(ruleAtoms.neg.begin(), ruleAtoms.neg.end(), trueAtoms.begin(), trueAtoms.end()) != ruleAtoms.neg.end()
					|| std::find_first_of(ruleAtoms.pos.begin(), ruleAtoms.pos.end(), falseAtoms.begin(), falseAtoms.end()) != ruleAtoms.pos.end())
				newTuple->rules.insert(introduced);

			std::pair<TupleSet::iterator, bool> result = newTuples->insert(TupleSet::value_type(newTuple, it->second));
			assert(result.second); (void)result; // In a rule introduction node, tuples cannot coincide
		}
	} else {
		// Atom introduction
		typedef std::pair<sharp::Vertex, const Problem::VerticesInRule*> RuleAndAtoms;
		std::vector<RuleAndAtoms> currentRules;
		foreach(sharp::Vertex v, node->getVertices()) {
			if(problem.vertexIsRule(v))
				currentRules.push_back(std::make_pair(v, &problem.getAtomsInRule(v)));
		}

		for(TupleSet::const_iterator it = childTuples->begin(); it != childTuples->end(); ++it) {
			Tuple& t = dynamic_cast<Tuple&>(*it->first);

			// 1. Guess true
			Tuple* newTuple = new Tuple(t);
			newTuple->atoms.insert(introduced);

			// Add thereby satisfied rules
			foreach(RuleAndAtoms& r, currentRules) {
				const Problem::VerticesInRule& rv = *r.second;
				if(std::find(rv.head.begin(), rv.head.end(), introduced) != rv.head.end() || std::find(rv.neg.begin(), rv.neg.end(), introduced) != rv.neg.end())
					newTuple->rules.insert(r.first);
			}

			// Insert new tuple (and, if necessary, combine coinciding tuples)
			sharp::Solution* s = addToSolution(it->second, introduced);
			std::pair<TupleSet::iterator, bool> result = newTuples->insert(TupleSet::value_type(newTuple, s));
			if(!result.second) {
				newTuples->erase(result.first);
				newTuples->insert(TupleSet::value_type(newTuple, combineSolutions(sharp::Union, result.first->second, s)));
			}

			// 2. Guess false
			newTuple = new Tuple(t);

			// Add thereby satisfied rules
			foreach(RuleAndAtoms& r, currentRules) {
				const Problem::VerticesInRule& rv = *r.second;
				if(std::find(rv.pos.begin(), rv.pos.end(), introduced) != rv.pos.end())
					newTuple->rules.insert(r.first);
			}

			// Insert new tuple (and, if necessary, combine coinciding tuples)
			result = newTuples->insert(TupleSet::value_type(newTuple, it->second));
			if(!result.second) {
				newTuples->erase(result.first);
				newTuples->insert(TupleSet::value_type(newTuple, combineSolutions(sharp::Union, result.first->second, it->second)));
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

TupleSet* AlgorithmNormalized::evaluateRemovalNode(const ExtendedHypertree* node)
{
	TupleSet* childTuples = evaluateNode(node->firstChild());
#if NO_PROGRESS_REPORT > 0
	printProgressLine(node, childTuples->size());
#endif
#ifdef VERBOSE
	printBagContents(node->getVertices());
#endif
	TupleSet* newTuples = new TupleSet;
	sharp::Vertex removed = node->getDifference();

	if(problem.vertexIsRule(removed)) {
		for(TupleSet::const_iterator it = childTuples->begin(); it != childTuples->end(); ++it) {
			Tuple& t = dynamic_cast<Tuple&>(*it->first);
			if(std::find(t.rules.begin(), t.rules.end(), removed) == t.rules.end())
				continue;

			Tuple* newTuple = new Tuple(t);
			newTuple->rules.erase(removed);

			std::pair<TupleSet::iterator, bool> result = newTuples->insert(TupleSet::value_type(newTuple, it->second));
			assert(result.second); (void)result; // In a rule removal node, tuples cannot coincide
		}
	} else {
		// Atom removal
		for(TupleSet::const_iterator it = childTuples->begin(); it != childTuples->end(); ++it) {
			Tuple& t = dynamic_cast<Tuple&>(*it->first);

			Tuple* newTuple = new Tuple(t);
			newTuple->atoms.erase(removed);

			std::pair<TupleSet::iterator, bool> result = newTuples->insert(TupleSet::value_type(newTuple, it->second));
			if(!result.second) {
				newTuples->erase(result.first);
				newTuples->insert(TupleSet::value_type(newTuple, combineSolutions(sharp::Union, result.first->second, it->second)));
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
