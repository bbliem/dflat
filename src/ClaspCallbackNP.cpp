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
//#define foreach BOOST_FOREACH // XXX: Strange: After a Boost upgrade, this led to "error: 'boost::BOOST_FOREACH' has not been declared". Moving it down helps...

#include "ClaspCallbackNP.h"
#include "GringoOutputProcessor.h"
#include "TupleNP.h"

#define foreach BOOST_FOREACH

void ClaspCallbackNP::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackNP::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve && e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			foreach(const GringoOutputProcessor::MapAtom& it, gringoOutput.getMapAtoms()) {
#ifndef DISABLE_ANSWER_SET_CHECKS
				if(it.level != 0)
					throw std::runtime_error("map predicate uses invalid level");
#endif
				mapAtoms.push_back(MapAtom(it.vertex, it.value, symTab[it.symbolTableKey].lit));
			}

			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleAtoms())
				chosenChildTupleAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleLAtoms()) // XXX: Obsolete
				chosenChildTupleLAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleRAtoms()) // XXX: Obsolete
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

	std::vector<const sharp::TupleTable::value_type*> childTuplesAndPlans;
	childTuplesAndPlans.reserve(numChildNodes);

	foreach(const LongToLiteral::value_type& it, chosenChildTupleAtoms) {
		if(s.isTrue(it.second)) {
			childTuplesAndPlans.push_back(reinterpret_cast<const sharp::TupleTable::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childTuplesAndPlans.size() == numChildNodes)
				break;
#endif
		}
	}

	// XXX: Obsolete
	foreach(const LongToLiteral::value_type& it, chosenChildTupleLAtoms) {
		if(s.isTrue(it.second)) {
			childTuplesAndPlans.push_back(reinterpret_cast<const sharp::TupleTable::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childTuplesAndPlans.size() == numChildNodes)
				break;
#else
			if(childTuplesAndPlans.size() != 1)
				throw std::runtime_error("You may only use chosenChildTuple/1 if you use neither chosenChildTupleL/1 nor chosenChildTupleR/1.");
#endif
		}
	}

	// XXX: Obsolete
	foreach(const LongToLiteral::value_type& it, chosenChildTupleRAtoms) {
		if(s.isTrue(it.second)) {
			childTuplesAndPlans.push_back(reinterpret_cast<const sharp::TupleTable::value_type*>(it.first));
#ifdef DISABLE_ANSWER_SET_CHECKS
			if(childTuplesAndPlans.size() == numChildNodes)
				break;
#else
			if(childTuplesAndPlans.size() != 2)
				throw std::runtime_error("You may only use chosenChildTuple/1 if you use neither chosenChildTupleL/1 nor chosenChildTupleR/1.");
#endif
		}
	}

#ifndef DISABLE_ANSWER_SET_CHECKS
	if(childTuplesAndPlans.size() > 0 && childTuplesAndPlans.size() != numChildNodes)
		throw std::runtime_error("Number of chosen child tuples not equal to number of child nodes");
#endif

	foreach(const LongToLiteral::value_type& it, currentCostAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(newTuple.currentCost != 0)
				throw std::runtime_error("Multiple current costs");
#endif
			newTuple.currentCost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(newTuple.cost != 0)
				throw std::runtime_error("Multiple costs");
#endif
			newTuple.cost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(MapAtom& atom, mapAtoms) {
		if(s.isTrue(atom.literal)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			// Only current vertices may be assigned
			if(currentVertices.find(atom.vertex) == currentVertices.end()) {
				std::ostringstream err;
				err << "Attempted assigning non-current vertex " << atom.vertex;
				throw std::runtime_error(err.str());
			}

			// vertex must not be assigned yet
			if(newTuple.assignment.find(atom.vertex) != newTuple.assignment.end()) {
				std::ostringstream err;
				err << "Multiple assignments to vertex " << atom.vertex;
				throw std::runtime_error(err.str());
			}
#endif
			newTuple.assignment[atom.vertex] = atom.value;
		}
	}
#ifndef DISABLE_ANSWER_SET_CHECKS
	// All vertices must be assigned now
	std::set<std::string> assigned;
	foreach(const Tuple::Assignment::value_type& kv, newTuple.assignment)
		assigned.insert(kv.first);
	if(assigned != currentVertices)
		throw std::runtime_error("Not all current vertices have been assigned");
#endif

//	if(oldTupleAndPlan) {
//		// This is an exchange node
//		algorithm.addRowToTupleTable(tupleTable, &newTuple,
//				algorithm.getPlanFactory().extend(oldTupleAndPlan->second, newTuple));
//	} else if(leftTupleAndPlan && rightTupleAndPlan) {
//		// This is a join node
//		algorithm.addRowToTupleTable(tupleTable, &newTuple,
//				algorithm.getPlanFactory().join(leftTupleAndPlan->second, rightTupleAndPlan->second, newTuple));
//	} else {
//		assert(!oldTupleAndPlan && !leftTupleAndPlan && !rightTupleAndPlan);
//		// This is a leaf node (or we don't have chosenChildTuples because we only solve the decision problem)
//		algorithm.addRowToTupleTable(tupleTable, &newTuple,
//				algorithm.getPlanFactory().leaf(newTuple));
//	}

	sharp::Plan* plan;
	if(childTuplesAndPlans.empty())
		plan = algorithm.getPlanFactory().leaf(newTuple);
	else if(childTuplesAndPlans.size() == 1)
		plan = algorithm.getPlanFactory().join(newTuple, childTuplesAndPlans[0]->second);
	else {
		plan = algorithm.getPlanFactory().join(newTuple, childTuplesAndPlans[0]->second, childTuplesAndPlans[1]->second);
		for(unsigned i = 2; i < childTuplesAndPlans.size(); ++i)
			plan = algorithm.getPlanFactory().join(newTuple, plan, childTuplesAndPlans[i]->second);
	}
	algorithm.addRowToTupleTable(tupleTable, &newTuple, plan);
}
