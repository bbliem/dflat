#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ModelProcessor.h"
#include "Tuple.h"
#include "Algorithm.h"
#include "GringoOutputProcessor.h"
#include "Problem.h"

using sharp::VertexSet;
using sharp::TupleSet;
using sharp::Solution;
using sharp::Vertex;

void ModelProcessor::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ModelProcessor::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_state_enter || f.state() != Clasp::ClaspFacade::state_solve)
		return;

	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getMAtoms()) {
		mAtoms.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
	}
	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getChosenOldMAtoms()) {
		chosenOldMAtoms.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
	}
}

void ModelProcessor::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifndef NDEBUG
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

	const TupleSet::value_type* oldTupleAndSolution = 0;
	Tuple& newTuple = *new Tuple;

	foreach(LongAndLiteral& it, mAtoms) {
		if(s.isTrue(it.second))
			newTuple.m.insert(it.first);
	}
	foreach(LongAndLiteral& it, chosenOldMAtoms) {
		if(s.isTrue(it.second)) {
			assert(!oldTupleAndSolution);
			oldTupleAndSolution = reinterpret_cast<const TupleSet::value_type*>(it.first);
		}
	}

	Solution* newSolution;

	// TODO: Add to the old solution those introduced atoms that
	// were guessed to true.
	// In the meantime we just add all X s. t. m(X) is in the model, but
	// this adds atoms multiple times.
	VertexSet add;
	foreach(Vertex v, newTuple.m) {
		if(dynamic_cast<Problem*>(algorithm.problem())->vertexIsRule(v) == false)
			add.insert(v);
	}

	if(oldTupleAndSolution)
		newSolution = algorithm.addToSolution(oldTupleAndSolution->second, add);
	else
		newSolution = algorithm.createLeafSolution(add);

	algorithm.addToTupleSet(&newTuple, newSolution, &newTuples);
}
