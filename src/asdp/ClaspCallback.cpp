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
		}
		else if(e == Clasp::ClaspFacade::event_state_exit) {
			// For each old tuple, build new tuples from our collected paths
			foreach(const OldTuplesToPathsMap::value_type& it, oldTuplesToPaths) {
				const sharp::TupleSet::value_type* oldTupleAndSolution = it.first;
				foreach(const TopLevelAssignmentToPaths::value_type& it2, it.second) {
					const std::list<Path>& paths = it2.second;

					Tuple& newTuple = *new Tuple;

					foreach(const Path& path, paths) {
						assert(path.size() == numLevels);
						assert(path[0] == it2.first); // top-level assignment must coincide
						newTuple.tree.addPath(path.begin(), path.end());
						assert(newTuple.tree.children.size() == 1); // each tuple may only have one top-level assignment
					}

					sharp::VertexSet dummy; // TODO: Workaround since we only solve the decision problem at the moment
					sharp::Solution* newSolution = const_cast<ClaspAlgorithm&>(algorithm).createLeafSolution(dummy);
					const_cast<ClaspAlgorithm&>(algorithm).addToTupleSet(&newTuple, newSolution, &tupleSet);
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

	const sharp::TupleSet::value_type* oldTupleAndSolution = 0;
	foreach(const LongToLiteral::value_type& it, chosenChildTupleAtoms) {
		if(s.isTrue(it.second)) {
			assert(!oldTupleAndSolution);
			oldTupleAndSolution = reinterpret_cast<const sharp::TupleSet::value_type*>(it.first);
#ifdef NDEBUG // ifndef NDEBUG we want to check the assertion above
			break;
#endif
		}
	}

	Path path(numLevels);
	foreach(MapAtom& atom, mapAtoms) {
		assert(atom.level < numLevels);
		if(s.isTrue(atom.literal)) {
			assert(path[atom.level].find(atom.vertex) == path[atom.level].end()); // vertex must not be assigned yet
			path[atom.level][atom.vertex] = atom.value;
		}
	}
	oldTuplesToPaths[oldTupleAndSolution][path[0]].push_back(path);
}

} // namespace asdp
