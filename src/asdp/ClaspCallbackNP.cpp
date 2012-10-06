#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallbackNP.h"
#include "GringoOutputProcessor.h"
#include "TupleNP.h"

namespace asdp {

void ClaspCallbackNP::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackNP::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			foreach(const GringoOutputProcessor::MapAtom& it, gringoOutput.getMapAtoms()) {
				assert(it.level == 0);
				mapAtoms.push_back(MapAtom(it.vertex, it.value, symTab[it.symbolTableKey].lit));
#ifndef NDEBUG
				vertices.insert(it.vertex);
#endif
			}

			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleAtoms())
				chosenChildTupleAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleLAtoms())
				chosenChildTupleLAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleRAtoms())
				chosenChildTupleRAtoms[it.first] = symTab[it.second].lit;
	}
}

void ClaspCallbackNP::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
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

	TupleNP& newTuple = *new TupleNP;

	const sharp::TupleSet::value_type* oldTupleAndSolution = 0;
	const sharp::TupleSet::value_type* leftTupleAndSolution = 0;
	const sharp::TupleSet::value_type* rightTupleAndSolution = 0;

	foreach(const LongToLiteral::value_type& it, chosenChildTupleAtoms) {
		if(s.isTrue(it.second)) {
			assert(!oldTupleAndSolution);
			oldTupleAndSolution = reinterpret_cast<const sharp::TupleSet::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, chosenChildTupleLAtoms) {
		if(s.isTrue(it.second)) {
			assert(!leftTupleAndSolution);
			leftTupleAndSolution = reinterpret_cast<const sharp::TupleSet::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, chosenChildTupleRAtoms) {
		if(s.isTrue(it.second)) {
			assert(!rightTupleAndSolution);
			rightTupleAndSolution = reinterpret_cast<const sharp::TupleSet::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(MapAtom& atom, mapAtoms) {
		if(s.isTrue(atom.literal)) {
			assert(newTuple.assignments.find(atom.vertex) == newTuple.assignments.end()); // vertex must not be assigned yet
			newTuple.assignments[atom.vertex] = atom.value;
		}
	}
#ifndef NDEBUG
	// All vertices must be assigned now
	assert(vertices.size() == newTuple.assignments.size());
#endif

	// If oldTupleAndSolution is set, then left/rightTupleAndSolution are unset
	assert(!oldTupleAndSolution || (!leftTupleAndSolution && !rightTupleAndSolution));
	// If left/rightTupleAndSolution are set, then oldTupleAndSolution is unset
	assert(!(leftTupleAndSolution && rightTupleAndSolution) || !oldTupleAndSolution);

	sharp::VertexSet dummy; // TODO: Workaround since we only solve the decision problem at the moment
	sharp::Solution* newSolution = const_cast<ClaspAlgorithm&>(algorithm).createLeafSolution(dummy);
	const_cast<ClaspAlgorithm&>(algorithm).addToTupleSet(&newTuple, newSolution, &tupleSet);
}

} // namespace asdp
