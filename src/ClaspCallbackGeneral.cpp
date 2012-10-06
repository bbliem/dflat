#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallbackGeneral.h"
#include "GringoOutputProcessor.h"
#include "TupleGeneral.h"

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
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleLAtoms())
				chosenChildTupleLAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getChosenChildTupleRAtoms())
				chosenChildTupleRAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCurrentCostAtoms())
				currentCostAtoms[it.first] = symTab[it.second].lit;
			foreach(const GringoOutputProcessor::LongToSymbolTableKey::value_type& it, gringoOutput.getCostAtoms())
				costAtoms[it.first] = symTab[it.second].lit;
		}
		else if(e == Clasp::ClaspFacade::event_state_exit)
			pathCollection.fillTupleTable(tupleTable, algorithm);
	}
}

void ClaspCallbackGeneral::event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
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

	const sharp::TupleTable::value_type* oldTupleAndPlan = 0;
	const sharp::TupleTable::value_type* leftTupleAndPlan = 0;
	const sharp::TupleTable::value_type* rightTupleAndPlan = 0;
	unsigned currentCost = 0;
	unsigned cost = 0;

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
			assert(currentCost == 0);
			currentCost = it.first;
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	foreach(const LongToLiteral::value_type& it, costAtoms) {
		if(s.isTrue(it.second)) {
			assert(cost == 0);
			cost = it.first;
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	// If oldTupleAndPlan is set, then left/rightTupleAndPlan are unset
	assert(!oldTupleAndPlan || (!leftTupleAndPlan && !rightTupleAndPlan));
	// If left/rightTupleAndPlan are set, then oldTupleAndPlan is unset
	assert(!(leftTupleAndPlan && rightTupleAndPlan) || !oldTupleAndPlan);

	Path path(numLevels);
	foreach(MapAtom& atom, mapAtoms) {
		assert(atom.level < numLevels);
		if(s.isTrue(atom.literal)) {
#ifndef NDEBUG
			// Only current vertices may be assigned
			if(currentVertices.find(atom.vertex) == currentVertices.end()) {
				std::ostringstream err;
				err << "Attempted assigning non-current vertex " << atom.vertex;
				throw std::runtime_error(err.str());
			}
#endif
			assert(path[atom.level].find(atom.vertex) == path[atom.level].end()); // vertex must not be assigned yet
			path[atom.level][atom.vertex] = atom.value;
		}
	}
	assert(path.size() == numLevels);

#ifndef NDEBUG
	// On each level, all vertices must be assigned now
	foreach(const Tuple::Assignment& levelAssignment, path) {
		std::set<std::string> assigned;
		foreach(const Tuple::Assignment::value_type& kv, levelAssignment)
			assigned.insert(kv.first);
		assert(assigned == currentVertices);
	}
#endif

	if(oldTupleAndPlan)
		leftTupleAndPlan = oldTupleAndPlan;
	pathCollection.insert(path, leftTupleAndPlan, rightTupleAndPlan, currentCost, cost);
}

inline void ClaspCallbackGeneral::PathCollection::insert(const Path& path, const TableRow* leftPredecessor, const TableRow* rightPredecessor, unsigned currentCost, unsigned cost)
{
	assert(!path.empty());
	TopLevelAssignmentToTupleData& tupleDataMap = predecessorData[TableRowPair(leftPredecessor, rightPredecessor)];
	const Tuple::Assignment& topLevelAssignment = path.front();
	TupleData& tupleData = tupleDataMap[topLevelAssignment];

	tupleData.paths.push_back(path);
	assert(tupleData.currentCost == 0 || tupleData.currentCost == currentCost);
	assert(tupleData.cost == 0 || tupleData.cost == cost);
	tupleData.currentCost = currentCost;
	tupleData.cost = cost;
}

inline void ClaspCallbackGeneral::PathCollection::fillTupleTable(sharp::TupleTable& tupleTable, const Algorithm& algorithm) const
{
	// For all (pairs of) predecessors, build new tuples from our collected paths
	foreach(const PredecessorData::value_type& it, predecessorData) {
		TableRowPair predecessors = it.first;
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

			if(predecessors.second) {
				// This is a join node
				assert(predecessors.first);
				algorithm.addRowToTupleTable(tupleTable, &newTuple,
						algorithm.getPlanFactory().join(predecessors.first->second, predecessors.second->second, newTuple));
			} else if(predecessors.first) {
				// This is an exchange node
				algorithm.addRowToTupleTable(tupleTable, &newTuple,
						algorithm.getPlanFactory().extend(predecessors.first->second, newTuple));
			} else {
				// This is a leaf node (or we don't have chosenChildTuples because we only solve the decision problem)
				algorithm.addRowToTupleTable(tupleTable, &newTuple,
						algorithm.getPlanFactory().leaf(newTuple));
			}
		}
	}
}
