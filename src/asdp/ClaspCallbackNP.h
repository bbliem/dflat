#pragma once

#include <clasp/clasp_facade.h>

#include "ClaspAlgorithm.h"

namespace asdp {

class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ClaspCallbackNP : public Clasp::ClaspFacade::Callback
{
public:
	ClaspCallbackNP(const ClaspAlgorithm& algorithm, sharp::TupleSet& tupleSet, const GringoOutputProcessor& gringoOutput)
		: algorithm(algorithm), tupleSet(tupleSet), gringoOutput(gringoOutput)
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

	// cf. GringoOutputProcessor.h
	struct MapAtom {
		std::string vertex;
		std::string value;
		Clasp::Literal literal;
		MapAtom(const std::string& vertex, const std::string& value, Clasp::Literal literal)
			: vertex(vertex), value(value), literal(literal)
		{}
	};
	std::vector<MapAtom> mapAtoms;
	typedef std::map<long, Clasp::Literal> LongToLiteral;
	LongToLiteral chosenChildTupleAtoms;

#ifndef NDEBUG
	std::set<std::string> vertices; // To check if all vertices are assigned
#endif
};

} // namespace asdp
