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

#include "ClaspCallbackGeneral.h"
#include "GringoOutputProcessor.h"
#include "TupleGeneral.h"

#define foreach BOOST_FOREACH

void ClaspCallbackGeneral::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallbackGeneral::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
{
	if(f.state() == Clasp::ClaspFacade::state_solve) {
		if(e == Clasp::ClaspFacade::event_state_enter) {
			Clasp::SymbolTable& symTab = f.config()->ctx.symTab();

			foreach(const GringoOutputProcessor::MapAtom& it, gringoOutput.getMapAtoms())
				mapAtoms.push_back(MapAtom(it.level, it.vertex, it.value, symTab[it.symbolTableKey].lit));
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
		else if(e == Clasp::ClaspFacade::event_state_exit)
			pathCollection.fillTupleTable(tupleTable, algorithm);
	}
}

void ClaspCallbackGeneral::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
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

	std::vector<const sharp::TupleTable::value_type*> childTuplesAndPlans;
	childTuplesAndPlans.reserve(numChildNodes);
	unsigned currentCost = 0;
	unsigned cost = 0;

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
			if(currentCost != 0)
				throw std::runtime_error("Multiple current costs");
#endif
			currentCost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(cost != 0)
				throw std::runtime_error("Multiple costs");
#endif
			cost = it.first;
#ifndef DISABLE_ANSWER_SET_CHECKS // Otherwise we want to check the condition above
			break;
#endif
		}
	}

//#ifndef DISABLE_ANSWER_SET_CHECKS
// TODO: Check if for each child node we have a chosenChildTuple (or none at all)
//#endif

	Path path(numLevels);
	unsigned int highestLevel = 0; // Highest level of an assignment encountered so far
	foreach(MapAtom& atom, mapAtoms) {
		if(s.isTrue(atom.literal)) {
			highestLevel = std::max(highestLevel, atom.level);
#ifndef DISABLE_ANSWER_SET_CHECKS
			if(atom.level >= numLevels) {
				std::ostringstream err;
				err << "map predicate uses invalid level " << atom.level;
				throw std::runtime_error(err.str());
			}

			if(currentVertices.find(atom.vertex) == currentVertices.end()) {
				std::ostringstream err;
				err << "Attempted assigning non-current vertex " << atom.vertex << " on level " << atom.level;
				throw std::runtime_error(err.str());
			}

			if(path[atom.level].find(atom.vertex) != path[atom.level].end()) {
				std::ostringstream err;
				err << "Multiple assignments to vertex " << atom.vertex << " on level " << atom.level;
				throw std::runtime_error(err.str());
			}
#endif
			path[atom.level][atom.vertex] = atom.value;
		}
	}
	// A path does not have to use all levels, but up to the highest used level it must be connected.
	path.resize(highestLevel+1);
#ifndef DISABLE_ANSWER_SET_CHECKS
	// On each assignment of the path, all current vertices must be assigned
	unsigned int l = 0;
	foreach(const Tuple::Assignment& levelAssignment, path) {
		std::set<std::string> assigned;
		foreach(const Tuple::Assignment::value_type& kv, levelAssignment)
			assigned.insert(kv.first);
		if(assigned != currentVertices) {
			std::ostringstream err;
			err << "Not all current vertices have been assigned on level " << l;
			throw std::runtime_error(err.str());
		}
		++l;
	}
#endif

	pathCollection.insert(path, childTuplesAndPlans, currentCost, cost);
}

inline void ClaspCallbackGeneral::PathCollection::insert(const Path& path, const std::vector<const TableRow*>& predecessors, unsigned currentCost, unsigned cost)
{
	assert(!path.empty());
	TopLevelAssignmentToTupleData& tupleDataMap = predecessorData[predecessors];
	const Tuple::Assignment& topLevelAssignment = path.front();
	TupleData& tupleData = tupleDataMap[topLevelAssignment];

	tupleData.paths.push_back(path);
#ifndef DISABLE_ANSWER_SET_CHECKS
	if(tupleData.currentCost != 0 && tupleData.currentCost != currentCost)
		throw std::runtime_error("Different current cost for same top-level assignment");
	if(tupleData.cost != 0 && tupleData.cost != cost)
		throw std::runtime_error("Different cost for same top-level assignment");
#endif
	tupleData.currentCost = currentCost;
	tupleData.cost = cost;
}

inline void ClaspCallbackGeneral::PathCollection::fillTupleTable(sharp::TupleTable& tupleTable, const Algorithm& algorithm) const
{
	// For all (pairs of) predecessors, build new tuples from our collected paths
	foreach(const PredecessorData::value_type& it, predecessorData) {
		const TableRows& predecessors = it.first;
		foreach(const TopLevelAssignmentToTupleData::value_type& it2, it.second) {
			const TupleData& tupleData = it2.second;

			TupleGeneral& newTuple = *new TupleGeneral;
			newTuple.currentCost = tupleData.currentCost;
			newTuple.cost = tupleData.cost;

			foreach(const Path& path, tupleData.paths) {
				assert(path.front() == it2.first); // top-level assignment must coincide
				newTuple.tree.addPath(path.begin(), path.end());
				assert(newTuple.tree.children.size() == 1); // each tuple may only have one top-level assignment
			}

//			std::vector<const sharp::Plan*> plans;
//			plans.reserve(predecessors.size());
//			foreach(const TableRow* row, predecessors)
//				plans.push_back(row->second);
//			algorithm.addRowToTupleTable(tupleTable, &newTuple, algorithm.getPlanFactory().join(newTuple, plans));

			sharp::Plan* plan;
			if(predecessors.empty())
				plan = algorithm.getPlanFactory().leaf(newTuple);
			else if(predecessors.size() == 1)
				plan = algorithm.getPlanFactory().join(newTuple, predecessors[0]->second);
			else {
				plan = algorithm.getPlanFactory().join(newTuple, predecessors[0]->second, predecessors[1]->second);
				for(unsigned i = 2; i < predecessors.size(); ++i)
					plan = algorithm.getPlanFactory().join(newTuple, plan, predecessors[i]->second);
			}
			algorithm.addRowToTupleTable(tupleTable, &newTuple, plan);
		}
	}
}
