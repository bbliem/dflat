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
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
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

	const sharp::TupleTable::value_type* oldTupleAndPlan = 0;
	const sharp::TupleTable::value_type* leftTupleAndPlan = 0;
	const sharp::TupleTable::value_type* rightTupleAndPlan = 0;

	foreach(const LongToLiteral::value_type& it, chosenChildTupleAtoms) {
		if(s.isTrue(it.second)) {
			assert(!oldTupleAndPlan);
			oldTupleAndPlan = reinterpret_cast<const sharp::TupleTable::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, chosenChildTupleLAtoms) {
		if(s.isTrue(it.second)) {
			assert(!leftTupleAndPlan);
			leftTupleAndPlan = reinterpret_cast<const sharp::TupleTable::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, chosenChildTupleRAtoms) {
		if(s.isTrue(it.second)) {
			assert(!rightTupleAndPlan);
			rightTupleAndPlan = reinterpret_cast<const sharp::TupleTable::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
			assert(newTuple.currentCost == 0);
			newTuple.currentCost = it.first;
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
			assert(newTuple.cost == 0);
			newTuple.cost = it.first;
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(MapAtom& atom, mapAtoms) {
		if(s.isTrue(atom.literal)) {
			assert(newTuple.assignment.find(atom.vertex) == newTuple.assignment.end()); // vertex must not be assigned yet
			newTuple.assignment[atom.vertex] = atom.value;
		}
	}
#ifndef NDEBUG
	// All vertices must be assigned now
	assert(vertices.size() == newTuple.assignment.size());
#endif

	// If oldTupleAndPlan is set, then left/rightTupleAndPlan are unset
	assert(!oldTupleAndPlan || (!leftTupleAndPlan && !rightTupleAndPlan));
	// If left/rightTupleAndPlan are set, then oldTupleAndPlan is unset
	assert(!(leftTupleAndPlan && rightTupleAndPlan) || !oldTupleAndPlan);

	if(oldTupleAndPlan) {
		// This is an exchange node
		algorithm.addRowToTupleTable(tupleTable, &newTuple,
				algorithm.getPlanFactory().extend(oldTupleAndPlan->second, newTuple));
	} else if(leftTupleAndPlan && rightTupleAndPlan) {
		// This is a join node
		algorithm.addRowToTupleTable(tupleTable, &newTuple,
				algorithm.getPlanFactory().join(leftTupleAndPlan->second, rightTupleAndPlan->second, newTuple));
	} else {
		assert(!oldTupleAndPlan && !leftTupleAndPlan && !rightTupleAndPlan);
		// This is a leaf node (or we don't have chosenChildTuples because we only solve the decision problem)
		algorithm.addRowToTupleTable(tupleTable, &newTuple,
				algorithm.getPlanFactory().leaf(newTuple));
	}
}

} // namespace asdp
