#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallback.h"
#include "GringoOutputProcessor.h"

namespace cyclic_ordering {

void ClaspCallback::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_state_enter || f.state() != Clasp::ClaspFacade::state_solve)
		return;

	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

	foreach(const GringoOutputProcessor::MappingAndSymbolTableKey& it, gringoOutput.getMap())
		map.push_back(MappingAndLiteral(it.first, symTab[it.second].lit));
}

void ClaspCallback::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifdef VERBOSE
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

	Tuple& newTuple = *new Tuple;
	newTuple.ordering.resize(gringoOutput.getN());

	foreach(MappingAndLiteral& it, map)
		if(s.isTrue(it.second))
			newTuple.ordering[it.first.second-1] = it.first.first;

	sharp::VertexSet dummy; // XXX: Workaround since we only solve the decision problem at the moment
	sharp::Solution* newSolution = const_cast<ClaspAlgorithm&>(algorithm).createLeafSolution(dummy);
	const_cast<ClaspAlgorithm&>(algorithm).addToTupleSet(&newTuple, newSolution, &newTuples);
}

} // namespace cyclic_ordering
