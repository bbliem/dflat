#pragma once

#include <clasp/clasp_facade.h>

#include "ClaspAlgorithm.h"
#include "Tuple.h"

namespace asdp {

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallback : public Clasp::ClaspFacade::Callback
{
public:
	ClaspCallback(const ClaspAlgorithm& algorithm, sharp::TupleSet& tupleSet, const GringoOutputProcessor& gringoOutput, unsigned int level)
		: algorithm(algorithm), tupleSet(tupleSet), gringoOutput(gringoOutput), numLevels(level+1)
	{}

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg);

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&);

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f);

private:
	const ClaspAlgorithm& algorithm;
	sharp::TupleSet& tupleSet;
	const GringoOutputProcessor& gringoOutput;
	const unsigned int numLevels;

	// cf. GringoOutputProcessor.h
	struct MapAtom {
		unsigned int level;
		std::string vertex;
		std::string value;
		Clasp::Literal literal;
		MapAtom(unsigned int level, const std::string& vertex, const std::string& value, Clasp::Literal literal)
			: level(level), vertex(vertex), value(value), literal(literal)
		{}
	};
	std::vector<MapAtom> mapAtoms;
	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral chosenChildTupleAtoms;

	// Because one tuple can be constituted of multiple AS's, we cannot insert new tuples upon arrival of a new AS but must rather collect all AS data until the solve state is finished.
	typedef std::vector<Tuple::Assignment> Path; // By "path" we denote a path from root to leaf in Tuple::Tree. Each AS characterizes exactly one path.
	typedef std::map<Tuple::Assignment, std::list<Path> > TopLevelAssignmentToPaths; // Maps an assignment to paths starting with that assignment
	typedef std::map<const sharp::TupleSet::value_type*, TopLevelAssignmentToPaths> OldTuplesToPathsMap;
	OldTuplesToPathsMap oldTuplesToPaths;
};

} // namespace asdp
