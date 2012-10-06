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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallbackNP.h"
#include "GringoOutputProcessor.h"
#include "TupleNP.h"

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
#ifdef PRINT_MODELS
 			std::cout << std::endl;
#endif
	}
}

void ClaspCallbackNP::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(e != Clasp::ClaspFacade::event_model)
		return;

#ifdef PRINT_MODELS
	Clasp::SymbolTable& symTab = f.config()->ctx.symTab();
	std::cout << "Model " << f.config()->ctx.enumerator()->enumerated << ": ";
	for(Clasp::SymbolTable::const_iterator it = symTab.begin(); it != symTab.end(); ++it) {
		if(s.isTrue(it->second.lit) && !it->second.name.empty())
			std::cout << it->second.name.c_str() << ' ';
	}
	std::cout << std::endl;
#endif

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
#ifndef DISABLE_ASSIGNMENT_CHECK
			// Only current vertices may be assigned
			if(currentVertices.find(atom.vertex) == currentVertices.end()) {
				std::ostringstream err;
				err << "Attempted assigning non-current vertex " << atom.vertex;
				throw std::runtime_error(err.str());
			}
#endif
			assert(newTuple.assignment.find(atom.vertex) == newTuple.assignment.end()); // vertex must not be assigned yet
			newTuple.assignment[atom.vertex] = atom.value;
		}
	}
#ifndef DISABLE_ASSIGNMENT_CHECK
	// All vertices must be assigned now
	std::set<std::string> assigned;
	foreach(const Tuple::Assignment::value_type& kv, newTuple.assignment)
		assigned.insert(kv.first);
	if(assigned != currentVertices)
		throw std::runtime_error("Not all current vertices have been assigned");
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
