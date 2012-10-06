#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallback.h"
#include "GringoOutputProcessor.h"

namespace threeCol {

using sharp::VertexSet;
using sharp::TupleSet;
using sharp::Solution;
using sharp::Vertex;

void ClaspCallback::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_state_enter || f.state() != Clasp::ClaspFacade::state_solve)
		return;

	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getR())
		r.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getG())
		g.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getB())
		b.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
	foreach(const GringoOutputProcessor::LongAndSymbolTableKey& it, gringoOutput.getChosenChildTuple())
		chosenChildTuple.push_back(LongAndLiteral(it.first, symTab[it.second].lit));
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

//#ifdef VERBOSE
//	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
//	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated << ": ";
//	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
//		if(s.isTrue(it->second.lit) && !it->second.name.empty())
//			std::cout << it->second.name.c_str() << ' ';
//	}
//	std::cout << std::endl;
//#endif

	const TupleSet::value_type* oldTupleAndSolution = 0;
	Tuple& newTuple = *new Tuple;

	foreach(LongAndLiteral& it, r)
		if(s.isTrue(it.second))
			newTuple.red.insert(it.first);
	foreach(LongAndLiteral& it, g)
		if(s.isTrue(it.second))
			newTuple.green.insert(it.first);
	foreach(LongAndLiteral& it, b)
		if(s.isTrue(it.second))
			newTuple.blue.insert(it.first);
	foreach(LongAndLiteral& it, chosenChildTuple) {
		if(s.isTrue(it.second)) {
			assert(!oldTupleAndSolution);
			oldTupleAndSolution = reinterpret_cast<const TupleSet::value_type*>(it.first);
		}
	}

	Solution* newSolution;
	VertexSet dummy; // XXX: SHARP only allows subsets of the vertices as solutions, but we need colorings. So we can't solve the enumeration problem at the moment. How to change this?

	if(oldTupleAndSolution)
		newSolution = const_cast<ClaspAlgorithm&>(algorithm).addToSolution(oldTupleAndSolution->second, dummy);
	else
		newSolution = const_cast<ClaspAlgorithm&>(algorithm).createLeafSolution(dummy);

	const_cast<ClaspAlgorithm&>(algorithm).addToTupleSet(&newTuple, newSolution, &newTuples);
}

} // namespace threeCol
