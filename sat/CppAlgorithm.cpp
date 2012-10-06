#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#if PROGRESS_REPORT > 2
#include <boost/timer/timer.hpp>
#endif

#include "CppAlgorithm.h"
#include "Problem.h"
#include "Tuple.h"
#include "../PowerSet.h"

namespace sat {

using sharp::TupleSet;
using sharp::VertexSet;
using sharp::Vertex;

CppAlgorithm::CppAlgorithm(Problem& problem)
	: ::Algorithm(problem)
{
}

CppAlgorithm::~CppAlgorithm()
{
}

TupleSet* CppAlgorithm::exchangeLeaf(const VertexSet& vertices, const VertexSet& introduced, const VertexSet& removed)
{
	VertexSet currentAtoms;
	std::map<Vertex,VertexSet> enforcedIfPos, enforcedIfNeg; // enforcedIfPos[a] is the set of rules that are made true by the atom a being true
	foreach(Vertex v, vertices) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v)) {
			const Problem::VerticesInRule& atoms = dynamic_cast<Problem&>(problem).getAtomsInRule(v);
			foreach(Vertex a, atoms.head)
				enforcedIfPos[a].insert(v);
			foreach(Vertex a, atoms.pos)
				enforcedIfNeg[a].insert(v);
			foreach(Vertex a, atoms.neg)
				enforcedIfPos[a].insert(v);
		} else
			currentAtoms.insert(v);
	}

	TupleSet* newTuples = new TupleSet;

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

		sharp::Solution* s = createLeafSolution(t.atoms);
		addToTupleSet(&t, s, newTuples);
	}
	return newTuples;
}

TupleSet* CppAlgorithm::exchangeNonLeaf(const VertexSet& vertices, const VertexSet& introduced, const VertexSet& removed, const TupleSet& childTuples)
{
	TupleSet* newTuples = new TupleSet;

	// XXX: Too many auxiliary sets...
	VertexSet removedAtoms, removedRules;
	foreach(Vertex v, removed) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
			removedRules.insert(v);
		else
			removedAtoms.insert(v);
	}
	VertexSet introducedAtoms, introducedRules;
	foreach(Vertex v, introduced) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v))
			introducedRules.insert(v);
		else
			introducedAtoms.insert(v);
	}
	VertexSet currentAtoms;
	std::map<Vertex,VertexSet> enforcedIfPos, enforcedIfNeg; // enforcedIfPos[a] is the set of rules that are made true by the atom a being true
	foreach(Vertex v, vertices) {
		if(dynamic_cast<Problem&>(problem).vertexIsRule(v)) {
			const Problem::VerticesInRule& atoms = dynamic_cast<Problem&>(problem).getAtomsInRule(v);
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

	for(TupleSet::const_iterator cit = childTuples.begin(); cit != childTuples.end(); ++cit) {
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
			sharp::Solution* s = addToSolution(cit->second, t.atoms);
			addToTupleSet(&t, s, newTuples);
		}
	}

	return newTuples;
}

} // namespace sat
