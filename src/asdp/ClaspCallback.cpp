#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "ClaspCallback.h"
#include "GringoOutputProcessor.h"

namespace asdp {

void ClaspCallback::warning(const char* msg)
{
	std::cerr << "clasp warning: " << msg << std::endl;
}

void ClaspCallback::state(Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f)
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
		}
		else if(e == Clasp::ClaspFacade::event_state_exit) {
			// For all (pairs of) predecessors, build new tuples from our collected paths
			foreach(const PredecessorsToPathsMap::value_type& it, predecessorsToPaths) {
				TableRowPair predecessors = it.first;
				foreach(const TopLevelAssignmentToPaths::value_type& it2, it.second) {
					const std::list<Path>& paths = it2.second;

					Tuple& newTuple = *new Tuple;

					foreach(const Path& path, paths) {
						assert(path.size() == numLevels);
						assert(path[0] == it2.first); // top-level assignment must coincide
						newTuple.tree.addPath(path.begin(), path.end());
						assert(newTuple.tree.children.size() == 1); // each tuple may only have one top-level assignment
					}

					if(predecessors.second) {
						// This is a join node
						algorithm.addRowToTupleTable(tupleTable, &newTuple,
								algorithm.getPlanFactory().join(predecessors.first->second, predecessors.second->second));
					} else {
						// This is an exchange node
						algorithm.addRowToTupleTable(tupleTable, &newTuple,
								algorithm.getPlanFactory().extend(predecessors.first->second, newTuple));
					}
				}
			}
		}
	}
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

	// If oldTupleAndPlan is set, then left/rightTupleAndPlan are unset
	assert(!oldTupleAndPlan || (!leftTupleAndPlan && !rightTupleAndPlan));
	// If left/rightTupleAndPlan are set, then oldTupleAndPlan is unset
	assert(!(leftTupleAndPlan && rightTupleAndPlan) || !oldTupleAndPlan);

	Path path(numLevels);
	foreach(MapAtom& atom, mapAtoms) {
		assert(atom.level < numLevels);
		if(s.isTrue(atom.literal)) {
			assert(path[atom.level].find(atom.vertex) == path[atom.level].end()); // vertex must not be assigned yet
			path[atom.level][atom.vertex] = atom.value;
		}
	}
	if(oldTupleAndPlan)
		predecessorsToPaths[TableRowPair(oldTupleAndPlan,0)][path[0]].push_back(path);
	else
		predecessorsToPaths[TableRowPair(leftTupleAndPlan,rightTupleAndPlan)][path[0]].push_back(path);
}

} // namespace asdp
