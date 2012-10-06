#pragma once

#include <clasp/clasp_facade.h>
#include <sharp/AbstractAlgorithm.hpp>

#include "Tuple.h"

class Algorithm;
class GringoOutputProcessor;

// Gets called by clasp whenever a model has been found
class ModelProcessor : public Clasp::ClaspFacade::Callback
{
public:
	ModelProcessor(Algorithm& algorithm, sharp::TupleSet& newTuples, const GringoOutputProcessor& gringoOutput)
		: algorithm(algorithm), newTuples(newTuples), gringoOutput(gringoOutput)
	{}

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg);

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&);

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f);

private:
	Algorithm& algorithm;
	sharp::TupleSet& newTuples;
	const GringoOutputProcessor& gringoOutput;

	typedef std::pair<long, Clasp::Literal> LongAndLiteral;
	typedef std::vector<LongAndLiteral> LongAndLiteralVec;
	LongAndLiteralVec mAtom;
	LongAndLiteralVec mRule;
	LongAndLiteralVec chosenOldM;
};
